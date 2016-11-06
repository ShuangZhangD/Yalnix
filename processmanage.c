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
    TracePrintf(1,"Enter Kernelfork\n");
    int rc;

    rc = CheckAvailableFrame(currProc);
    if (rc) return ERROR;
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

    int childPid = childPcb->pid;
    pte_t *childPtb = childPcb->usrPtb;
    pte_t *childKernelPtb = childPcb->krnlStackPtb;
    dblist* childTermChild = childPcb->terminatedchild;
    dblist* childChildren = childPcb->children;

    memcpy((void*) childPcb, (void*) parentPcb, sizeof(pcb_t));
    childPcb->pid = childPid;
    childPcb->usrPtb = childPtb;
    childPcb->krnlStackPtb = childKernelPtb;
    childPcb->terminatedchild = childTermChild;
    childPcb->children = childChildren;

    //5. Duplicate the content of the frames used by parent to the free frames of a child
    CopyUserProcess(parentPcb->usrPtb, childPcb->usrPtb);

    //6. Put child into parent's children queue
    lstnode* childNode = TurnPCBToNode(childPcb);
    insert_tail(childNode,parentPcb->children);

    //7. Assign the address of the parent process to the child 
    childPcb->parent = parentProc;

    //8. Put child proc into readyqueue
    enreadyqueue(childProc,readyqueue);

    //Flush All TLB because 1. Kernel Stack Mapping has changed 2. User Page Table has been written into register
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

    // 3.  Copy the current kernel context and content of kernel stack from parent to child 
    rc = KernelContextSwitch(MyForkKCS, (void*) parentProc, (void*)childProc);
    if (rc){
        TracePrintf(1,"MyCloneKCS in kernelfork failed.\n");
        return ERROR;
    }

    TracePrintf(1,"parent:%p, child:%p\n", parentPcb->usrPtb, childPcb->usrPtb);

    TracePrintf(1,"Exit Kernelfork\n");
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
    TracePrintf(1,"Enter KernelExit\n");
    int status = uctxt->regs[0];
    pcb_t* currPcb = TurnNodeToPCB(currProc);
    currPcb->exitstatus = status;

    // If the initProcess Exit, halt the program
    if(currPcb->pid == 2) {
        Halt();
        return ERROR;
    }

    terminateProcess(currProc);
    TracePrintf(1,"Exit KernelExit\n");
    return ERROR;
    
    // if(proc->parent != NULL){
    //     pcb_t* pcb = (pcb_t *) proc->parent->content;
    //     pcb->terminatedchild = listinit();
    
    //switchnext();

}

int kernelwait(UserContext *uctxt){
    TracePrintf(1,"Enter kernelwait\n");
    pcb_t *proc = TurnNodeToPCB(currProc);

    if (isemptylist(proc->children) && isemptylist(proc->terminatedchild)){
        return ERROR;
    }

    if (isemptylist(proc->terminatedchild)){
        enblockqueue(currProc,blockqueue);
        switchproc();        
    }


    if (!isemptylist(proc->terminatedchild)){
        lstnode* remove = remove_head(proc->terminatedchild);
        pcb_t* removeproc = TurnNodeToPCB(remove);
        uctxt->regs[0] = removeproc->exitstatus;
        free(remove);
        return removeproc->pid;
    }
    // else{
    //     enblockqueue(currProc,blockqueue);
    //     //TODO Block Queue
    //     lstnode *node = dereadyqueue(readyqueue);
    //     if (node != currProc){
    //         TracePrintf(1,"The first node of readyqueue should be the current process!\n");
    //         return ERROR;
    //     }

    //     lstnode *fstnode = firstnode(readyqueue);
    //     switchproc(node, fstnode);
    // }

}

int kernelgetpid(){
    return TurnNodeToPCB(currProc)->pid;
}

//Capture TRAP_CLOCK
//TODO: Implement round-robin process scheduling with CPU quantum per process of 1 clock tick.
void TrapClock(UserContext *uctxt){
    TracePrintf(1, "TrapClock called\n");

    // int rc = 0;
    if (!isemptylist(waitingqueue)){

        lstnode *traverse = waitingqueue->head->next;
        while(traverse != NULL && traverse->id != -1) {               
            pcb_t* proc = TurnNodeToPCB(traverse);
            if(proc->clock > 0){
                proc->clock--;
                TracePrintf(1, "proc->clock:%d\n", proc->clock);
            }
            if(proc->clock == 0){
                lstnode* traverseNode = dewaitingqueue(traverse,waitingqueue);
                enreadyqueue(traverseNode,readyqueue);
            }

            traverse = traverse->next;   
        }
        
    }
    if (!isemptylist(readyqueue)){
        switchproc();
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
        TracePrintf(1, "pid = %d\n", TurnNodeToPCB(currProc)->pid);
        enwaitingqueue(currProc,waitingqueue);
        proc->clock = clock_ticks;

        rc = switchproc();
        if (rc) {
            return ERROR;
        }
    }
}

/*
    SYSCALL END
*/


int CheckAvailableFrame(lstnode *cur_p){
    int i, cnt = 0;
    pcb_t *pcb = TurnNodeToPCB(cur_p);  

    for (i=0; i < MAX_PT_LEN; i++){
        if (VALID == pcb->usrPtb[i].valid){
            cnt++;
        }
    }

    if (freeframe_list->size <= cnt){
        return ERROR;
    }

    return SUCCESS;
}

void CopyUserProcess (pte_t* parentPtb, pte_t* childPtb){
    int i;
    //Use a safety margin page as a buffer of copying memory
    g_pageTableR0[SAFETY_MARGIN_PAGE].valid = VALID;
    g_pageTableR0[SAFETY_MARGIN_PAGE].prot = (PROT_READ | PROT_WRITE);

    for (i = 0; i < MAX_PT_LEN; i++){
        if (VALID == parentPtb[i].valid){
            childPtb[i].valid = parentPtb[i].valid;

            childPtb[i].prot = PROT_WRITE;
            
            //allocate a free frame to it
            lstnode *first = remove_head(freeframe_list);
            if (NULL == first) TracePrintf(1, "Remove_head in CopyUserProcess failed!");
            childPtb[i].pfn = first->id;
            
            g_pageTableR0[SAFETY_MARGIN_PAGE].pfn = childPtb[i].pfn;

            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

            unsigned int desAddr = SAFETY_MARGIN_PAGE << PAGESHIFT;
            unsigned int srcAddr = (i << PAGESHIFT) + VMEM_1_BASE;

            memcpy((void *)desAddr, (void *)srcAddr, PAGESIZE);

            childPtb[i].prot = parentPtb[i].prot;
        }
    }
  
    //Restore the buffer.
    g_pageTableR0[SAFETY_MARGIN_PAGE].valid = INVALID;
    g_pageTableR0[SAFETY_MARGIN_PAGE].prot = PROT_NONE;
    g_pageTableR0[SAFETY_MARGIN_PAGE].pfn = UNALLOCATED;

    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

    return;
}

int switchproc()
{
        TracePrintf(1,"Enter switchproc.\n");
        int rc = 0;

        // lstnode *switchOut = dereadyqueue(readyqueue);
        // if (switchOut != currProc){
        //     TracePrintf(1, "The first node of readyqueue should be the current process!\n");
        //     return;
        // }

        lstnode *switchIn = firstnode(readyqueue);
        
        if (TERMINATED ==  TurnNodeToPCB(currProc)->procState){
            rc = KernelContextSwitch(MyTerminateKCS, (void *) currProc, (void *) switchIn);
            if (rc) TracePrintf(1,"MyTerminateKCS in switchproc failed!\n");  

        } else if (!isemptylist(readyqueue)){
            TracePrintf(1, "currProc->id%d\n", TurnNodeToPCB(currProc)->pid );
            TracePrintf(1, "switchIn->id%d\n" ,TurnNodeToPCB(switchIn)->pid);
            rc = KernelContextSwitch(MyTrueKCS, (void *) currProc, (void *) switchIn);
            if (rc) TracePrintf(1,"MyTrueKCS in switchproc failed!\n");
        }
        else{
            rc =  1;
        }	

        return rc;
}

int switchnext()
{
// <<<<<<< HEAD
//     lstnode *node = dereadyqueue(readyqueue);
//     if (node != currProc){
//         TracePrintf(1,"The first node of readyqueue should be the current process!\n");
//         return ERROR;
//     }

//     lstnode *fstnode = firstnode(readyqueue);
//     switchproc(node, fstnode);
// =======
    // lstnode *node = dereadyqueue(readyqueue);
    // lstnode *fstnode = firstnode(readyqueue);
    switchproc();
// >>>>>>> kernelfork
}

void terminateProcess(lstnode *procnode){
    int i, rc;
    pcb_t* proc = TurnNodeToPCB(procnode);
    proc->procState = TERMINATED;

    switchproc();

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
    TracePrintf(1, "Enter enblockqueue\n");    
    pcb_t* proc = TurnNodeToPCB(procnode);

    if (proc == NULL){
        return ERROR;
    }
    proc->procState = WAITING;
    insert_tail(procnode, queue);

    TracePrintf(1, "Exit enblockqueue\n"); 
    return 0;
}

lstnode* deblockqueue(lstnode* waitingnode,dblist* queue)
{
    TracePrintf(1,"Enter deblockqueue\n");
    TracePrintf(1,"Exit deblockqueue\n");     
    return remove_node(TurnNodeToPCB(waitingnode)->pid,queue);
}

int enreaderwaitingqueue(lstnode* procnode,dblist* queue)
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

lstnode* dereaderwaitingqueue(dblist* queue)
{
    TracePrintf(1,"Enter dewaitingqueue\n");
    TracePrintf(1,"Exit dewaitingqueue\n");     
    return remove_head(queue);
}

int enwriterwaitingqueue(lstnode* procnode,dblist* queue)
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

lstnode* dewriterwaitingqueue(dblist* queue)
{
    TracePrintf(1,"Enter dewaitingqueue\n");
    TracePrintf(1,"Exit dewaitingqueue\n");     
    return remove_head(queue);
}


lstnode* TurnPCBToNode(pcb_t *pcb){
    traverselist(readyqueue);
    TracePrintf(1, "pid = %d\n", pcb->pid);
    lstnode *node = nodeinit(pcb->pid);
    TracePrintf(1, "Segmentation line ================\n");
    traverselist(readyqueue);
    node->content = (void *) pcb;

    return node;
}

pcb_t* TurnNodeToPCB(lstnode *node){
    
    pcb_t *pcb = (pcb_t *) node->content;
    if (!pcb) TracePrintf(1, "Turn Node To PCB Error!\n");
    return pcb;
}