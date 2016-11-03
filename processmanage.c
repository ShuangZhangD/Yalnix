#include "processmanage.h"
#include "selfdefinedstructure.h"

extern dblist* freeframe_list;
extern lstnode* currProc;

extern int g_pid;
extern int g_kStackStPage;
extern int g_kStackEdPage;
extern int g_pageNumOfStack;
/*
    SYSCALL
*/
int kernelfork(UserContext *uctxt){
    int rc;
    /*
        1. Initialize Empty Child Process (including initialize user page table, allocate free frame to it)
    */
    lstnode* childProc = InitProc(); //Empty Process
    lstnode* parentProc = currProc;

    pcb_t* childPcb = TurnNodeToPCB(childProc);
    pcb_t* parentPcb = TurnNodeToPCB(parentProc);

    //  2. Record the current user context
    parentPcb->uctxt = *uctxt;
    childPcb->uctxt = *uctxt;

    // 3.  Copy the current kernel context and content of kernel stack from parent to child 
    rc = KernelContextSwitch(MyCloneKCS, (void*) parentProc, (void*)childProc);
    if (rc){
        TracePrintf(1,"MyCloneKCS in kernelfork failed.\n");
        return ERROR;
    }

    //4. Duplicate the content of the frames used by parent to the free frames of a child
    CopyUserProcess(parentPcb->usrPtb, childPcb->usrPtb);

    //5. Put child into parent's children queue
    if (parentPcb->children == NULL){
        parentPcb->children = listinit();
    } 
    insert_tail(childProc,parentPcb->children);

    //6. Assign the address of the parent process to the child 
    childPcb->parent = parentProc;

    //7. Put child proc into readyqueue
    enreadyqueue(childProc,readyqueue);

    //8. Doing KernelContext Switch here, so the parent process will start from here
    rc = KernelContextSwitch(MyTrueKCS, (void*)parentProc, (void*)childProc);
    if (rc){
        TracePrintf(1,"MyTrueKCS in kernelfork failed.\n");
        return ERROR;
    }

    //9. if current process is childprocess, return 0, otherwise return pid
    if(currProc == childProc) {
        return 0;
    }
    else{
        return childPcb->pid;
    }

    return ERROR;
}

int kernelexec(UserContext *uctxt){
    pcb_t *proc = (pcb_t*)currProc->content;
    char* name = (char*) uctxt->regs[0];
    char** args = (char**) uctxt->regs[1];
    int rc = LoadProgram(name,args,currProc);
    //todo
    if (rc == 0)
    {
        return 0;
    }
    if (rc == ERROR)
    {
        return ERROR;
    }
}

int kernelexit(UserContext *uctxt){
    pcb_t *proc = (pcb_t*)currProc->content;
    int status = uctxt->regs[0];

    if(proc->pid == 2)
    {
        Halt();
    }

    lstnode* traverse = proc->children->head;
    while(traverse != NULL)
    {               
        pcb_t* proc = (pcb_t*) traverse->content;
        proc->parent = NULL;
        traverse = traverse->next;   
    }

    if(proc->parent != NULL)
    {
        pcb_t* pcb = (pcb_t *) proc->parent->content;
        pcb->terminatedchild = listinit();
        remove_node(proc->pid, proc->children);
        insert_tail(currProc,pcb->terminatedchild);
        dewaitingqueue(proc->parent,waitingqueue);
        enreadyqueue(proc->parent,readyqueue); 
    }


    proc->exitstatus = status;
    switchproc();

}

int kernelwait(UserContext *uctxt){
    pcb_t *proc = (pcb_t*)currProc->content;

    if (isemptylist(proc->children) && isemptylist(proc->terminatedchild))
    {
        return ERROR;
    }
    if (!isemptylist(proc->terminatedchild))
    {
        lstnode* remove = remove_head(proc->terminatedchild);
        pcb_t* removeproc = (pcb_t*) remove->content;
        uctxt->regs[0] = removeproc->exitstatus;
        return removeproc->pid;
    }
    else{
        enwaitingqueue(currProc,waitingqueue);
        switchproc();
    }

}

int kernelgetpid(){
    return TurnNodeToPCB(currProc)->pid;
}

/*
    SYSCALL END
*/

void CopyUserProcess (pte_t* parentPtb, pte_t* childPtb){
    int i;
    //Use a safety margin page as a buffer of copying memory
    g_pageTableR0[SAFETY_MARGIN_PAGE].valid = 1;
    g_pageTableR0[SAFETY_MARGIN_PAGE].prot = (PROT_READ | PROT_WRITE);

    for (i = 0; i < MAX_PT_LEN-1; i++){
        if (VALID == parentPtb[i].valid){
            g_pageTableR0[SAFETY_MARGIN_PAGE].pfn = childPtb[i].pfn;

            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

            unsigned int desAddr = SAFETY_MARGIN_PAGE << PAGESHIFT;
            unsigned int srcAddr = (parentPtb[i].pfn) << PAGESHIFT;

            memcpy((void *)desAddr, (void *)srcAddr, PAGESIZE);
        }
    }
  
    //Restore the buffer.
    g_pageTableR0[SAFETY_MARGIN_PAGE].valid = 0;
    g_pageTableR0[SAFETY_MARGIN_PAGE].prot = PROT_NONE;
    g_pageTableR0[SAFETY_MARGIN_PAGE].pfn = UNALLOCATED;

    return;
}

int switchproc()
{
        TracePrintf(1,"Enter switchproc.\n");
        int rc;
        lstnode *fstnode = firstnode(readyqueue);
        if (!isemptylist(readyqueue)){
            rc = KernelContextSwitch(MyTrueKCS, (void *) currProc, (void *) fstnode);
            return 0;
        }
        else{
            return 1;
        }	
}



void terminateProcess(lstnode *procnode){
    int i, rc;
    pcb_t* proc = TurnNodeToPCB(procnode);
    proc->procState = TERMINATED;

    dereadyqueue(readyqueue);
    switchproc();
    
    ummap(proc->usrPtb, 0, MAX_PT_LEN-1, INVALID, PROT_NONE);

    // Insert a node to terminated queue
    proc->procState = TERMINATED;
    insert_tail(procnode, terminatedqueue);

    //TODO Delete process or relocate process pool

}

int enreadyqueue(lstnode* procnode,dblist* queue)
{	
    TracePrintf(1, "Enter enreadyqueue\n");    
	pcb_t* proc = TurnNodeToPCB(procnode);

	if (proc == NULL){
		return ERROR;
	}
	proc->procState = READY;
    insert_tail(procnode,queue);

	return 0;
}

lstnode* dereadyqueue(dblist* queue)
{
	return remove_head(queue);
}

int enwaitingqueue(lstnode* procnode,dblist* queue)
{
    TracePrintf(1, "Enter enwaitingqueue\n");    
	pcb_t* proc = TurnNodeToPCB(procnode);

	if (proc == NULL){
		return ERROR;
	}
	proc->procState = WAITING;
	insert_tail(procnode, queue);

    TracePrintf(1, "Exit enwaitingqueue\n"); 
	return 0;
}

lstnode* dewaitingqueue(lstnode* waitingnode,dblist* queue)
{
	
	return remove_node(TurnNodeToPCB(waitingnode)->pid,queue);
}


lstnode* TurnPCBToNode(pcb_t *pcb){

    lstnode *node = nodeinit(pcb->pid);
    node->content = (void *) pcb;

    return node;
}

pcb_t* TurnNodeToPCB(lstnode *node){
    
    pcb_t *pcb = (pcb_t *) node->content;
    if (!pcb) TracePrintf(1, "Turn Node To PCB Error!\n");
    return pcb;
}