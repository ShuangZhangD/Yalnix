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
    //4. Copy the page table of parent to the child
    memcpy((void*) childPcb->usrPtb, (void*) parentPcb->usrPtb, sizeof(pte_t)*MAX_PT_LEN);

    //5. Duplicate the content of the frames used by parent to the free frames of a child
    CopyUserProcess(parentPcb->usrPtb, childPcb->usrPtb);

    //6. Put child into parent's children queue
    if (parentPcb->children == NULL){
        parentPcb->children = listinit();
    } 
    insert_tail(childProc,parentPcb->children);

    //7. Assign the address of the parent process to the child 
    childPcb->parent = parentProc;

    //8. Put child proc into readyqueue
    enreadyqueue(childProc,readyqueue);

    //9. Doing KernelContext Switch here, so the parent process will start from here
    rc = switchproc(parentProc, childProc);
    if (rc){
        TracePrintf(1,"MyTrueKCS in kernelfork failed.\n");
        return ERROR;
    }

    //10. if current process is childprocess, return 0, otherwise return pid
    if(currProc == childProc) {
        return 0;
    }
    else{
        return childPcb->pid;
    }

    return ERROR;
}

int kernelexec(UserContext *uctxt){
    lstnode* loadProc = currProc;
    pcb_t* loadPcb = TurnNodeToPCB(loadProc);

    char* name = (char*) uctxt->regs[0];
    char** args = (char**) uctxt->regs[1];
    int rc = LoadProgram(name,args,loadProc);
    
    *uctxt = loadPcb->uctxt;

    if (rc == ERROR) {
        return ERROR;
    } else if (rc == KILL){
        terminateProcess(loadProc);
        return ERROR;
    }
    return SUCCESS;
}

int kernelexit(UserContext *uctxt){
    
    int status = uctxt->regs[0];
    pcb_t* currPcb = TurnNodeToPCB(currProc);
    currPcb->exitstatus = status;

    // If the initProcess Exit, halt the program
    if(currPcb->pid == 2) {
        terminateProcess(currProc);
        Halt();
        return ERROR;
    }

    //When the orphans later exit, you need not save or report their exit status since there is no longer anybody to care.
    if (NULL != currPcb->parent){
        pcb_t* currParent = TurnNodeToPCB(currPcb->parent);
        if (NULL == currParent->terminatedchild){
            currParent->terminatedchild = listinit();
        } 
        remove_node(currPcb->pid, currParent->children);

        currPcb->procState = TERMINATED;

        free(currPcb->usrPtb);
        free(currPcb->krnlStackPtb); 
        free(currPcb->parent);        
        free(currPcb->children);
        free(currPcb->terminatedchild);

        pcb_t* copyPcb = (pcb_t*) malloc(sizeof(currPcb));
        memcpy(copyPcb, currPcb, sizeof(currProc));

        insert_tail(TurnPCBToNode(copyPcb),currParent->terminatedchild);
    } 
    // If it has children, they should run normally but without a parent
    lstnode* traverse = currPcb->children->head->next;
    while(traverse != NULL && traverse->id != -1) {
        pcb_t* proc = TurnNodeToPCB(traverse);
        proc->parent = NULL;             
        traverse = traverse->next;   
    }
    terminateProcess(currProc);
    return ERROR;
    
    // 
    // if(proc->parent != NULL){
    //     pcb_t* pcb = (pcb_t *) proc->parent->content;
    //     pcb->terminatedchild = listinit();
    
    if (NULL != currPcb->parent && NULL != search_node(TurnNodeToPCB(currPcb->parent)->pid,blockqueue))
    {
    deblockqueue(currPcb->parent,blockqueue);
    enreadyqueue(currPcb->parent,readyqueue);    
    }
    //switchnext();




}

int kernelwait(UserContext *uctxt){

    pcb_t *proc = TurnNodeToPCB(currProc);

    if (isemptylist(proc->children) && isemptylist(proc->terminatedchild)){
        return ERROR;
    }

    if (!isemptylist(proc->terminatedchild)){
        lstnode* remove = remove_head(proc->terminatedchild);
        pcb_t* removeproc = TurnNodeToPCB(remove);
        uctxt->regs[0] = removeproc->exitstatus;
        free(remove);
        return removeproc->pid;
    }
    else{
        //TODO Block Queue
        lstnode *node = dereadyqueue(readyqueue);
        if (node != currProc){
            TracePrintf(1,"The first node of readyqueue should be the current process!\n");
            return ERROR;
        }
        enblockqueue(currProc,blockqueue);
        lstnode *fstnode = firstnode(readyqueue);
        switchproc(node, fstnode);
    }

}

int kerneldelay(UserContext *uctxt){
    TracePrintf(1, "Enter KernelDelay\n");

    pcb_t *proc = TurnNodeToPCB(currProc);
    proc->uctxt = *uctxt;
    int clock_ticks = uctxt->regs[0];
    int rc;
    if (clock_ticks == 0){
        return 0;
    }
    else if(clock_ticks <= 0){
        return ERROR;
    }
    else{
        lstnode *node = dereadyqueue(readyqueue);
        if (node != currProc){
            TracePrintf(1,"The first node of readyqueue should be the current process!\n");
            return ERROR;
        }
        enwaitingqueue(currProc,waitingqueue);
        proc->clock = clock_ticks;

        lstnode *fstnode = firstnode(readyqueue);
        rc = switchproc(node, fstnode);
        if (rc) {
            return ERROR;
        }
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

int switchproc(lstnode* switchOut, lstnode* switchIn)
{
        TracePrintf(1,"Enter switchproc.\n");
        int rc;
        if (!isemptylist(readyqueue)){
            rc = KernelContextSwitch(MyTrueKCS, (void *) switchOut, (void *) switchIn);
            if (rc) TracePrintf(1,"KernelContextSwitch in switchproc failed!\n");
            return 0;
        }
        else{
            return 1;
        }	
}

int switchnext()
{
    lstnode *node = dereadyqueue(readyqueue);
    lstnode *fstnode = firstnode(readyqueue);
    switchproc(node, fstnode);
}

void terminateProcess(lstnode *procnode){
    int i, rc;
    pcb_t* proc = TurnNodeToPCB(procnode);
    proc->procState = TERMINATED;

    lstnode* node = dereadyqueue(readyqueue);
    if (procnode!=node || currProc != node){
            TracePrintf(1, "The first node of readyqueue should be the current process!\n");
            return;
    }

    lstnode *fstnode = firstnode(readyqueue);
    switchproc(node, fstnode);

    emptyregion1pagetable(proc->usrPtb);
    free(procnode);                      

    //TODO Delete process or relocate process pool
    return;
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
    TracePrintf(1,"Enter dewaitingqueue\n");
    TracePrintf(1,"Exit dewaitingqueue\n");   	
	return remove_node(TurnNodeToPCB(waitingnode)->pid,queue);
}

int enblockqueue(lstnode* procnode,dblist* queue)
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

lstnode* deblockqueue(lstnode* waitingnode,dblist* queue)
{
    TracePrintf(1,"Enter dewaitingqueue\n");
    TracePrintf(1,"Exit dewaitingqueue\n");     
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