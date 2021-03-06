#include "processmanage.h"
#include "selfdefinedstructure.h"

extern dblist* freeframe_list;
extern lstnode* currProc;

extern int g_pid;
extern int g_kStackStPage;
extern int g_kStackEdPage;
extern int g_pageNumOfStack;

extern dblist* lockqueue;
extern dblist* cvarqueue;
extern dblist* freeframe_list;

/*
   SYSCALL
 */
int KernelFork(UserContext *uctxt){
    TracePrintf(2,"Enter KernelFork\n");
    int rc;

    rc = CheckAvailableFrame(currProc);
    if (rc) {
        TracePrintf(1, "Error! There is no enough Free Frames!\n");
        return ERROR;
    }

    //Initialize Empty Child Process (including initialize user page table, allocate free frame to it)
    lstnode* childProc = InitProc(); //Empty Process
    lstnode* parentProc = currProc;

    pcb_t* childPcb = TurnNodeToPCB(childProc);
    pcb_t* parentPcb = TurnNodeToPCB(parentProc);

    //Record the current user context
    parentPcb->uctxt = *uctxt;
    childPcb->uctxt = *uctxt;

    //Temporary Save Variables of child process
    int childPid = childPcb->pid;
    pte_t *childPtb = childPcb->usrPtb;
    pte_t *childKernelPtb = childPcb->krnlStackPtb;
    dblist* childTermChild = childPcb->terminatedchild;
    dblist* childChildren = childPcb->children;

    //Copy the whole PCB from Parent to child
    memcpy((void*) childPcb, (void*) parentPcb, sizeof(pcb_t));

    //Restore variables to child process
    childPcb->pid = childPid;
    childPcb->usrPtb = childPtb;
    childPcb->krnlStackPtb = childKernelPtb;
    childPcb->terminatedchild = childTermChild;
    childPcb->children = childChildren;

    //Duplicate the content of the frames used by parent to the free frames of a child
    CopyUserProcess(parentPcb->usrPtb, childPcb->usrPtb);

    //Put child into parent's children queue
    lstnode* childNode = TurnPCBToNode(childPcb);
    insert_tail(childNode,parentPcb->children);

    //Assign the address of the parent process to the child 
    childPcb->parent = parentProc;

    //Put child proc into readyqueue
    enreadyqueue(childProc,readyqueue);

    //Flush All TLB because 1. Kernel Stack Mapping has changed 2. User Page Table has been written into register
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

    //Copy the current kernel context and content of kernel stack from parent to child 
    rc = KernelContextSwitch(MyForkKCS, (void*) parentProc, (void*)childProc);
    if (rc){
        TracePrintf(1,"MyCloneKCS in KernelFork failed.\n");
        return ERROR;
    }

    TracePrintf(3,"parent:%p, child:%p\n", parentPcb->usrPtb, childPcb->usrPtb);

    TracePrintf(2,"Exit KernelFork\n");

    //If current process is childprocess, return 0, otherwise return pid
    if(currProc == childProc) {
        return 0;
    }
    else{
        return childPcb->pid;
    }

    return ERROR;
}

int KernelExec(UserContext *uctxt){
    TracePrintf(2,"Enter KernelExec\n");
    lstnode* loadProc = currProc;
    pcb_t* loadPcb = TurnNodeToPCB(loadProc);

    char *name = (char*) uctxt->regs[0];
    char **args = (char**) uctxt->regs[1];

    int rc = LoadProgram(name,args,loadProc);
    if (rc == ERROR) {
        return ERROR;
    } else if (rc == KILL){
        ProcessExit();
        return ERROR;
    }

    *uctxt = loadPcb->uctxt;

    TracePrintf(2,"Exit KernelExec\n");
    return SUCCESS;
}

int KernelExit(UserContext *uctxt){
    TracePrintf(2,"Enter KernelExit\n");
    int status = uctxt->regs[0];

    pcb_t* currPcb = TurnNodeToPCB(currProc);
    currPcb->exitstatus = status;

    TracePrintf(2,"Exit KernelExit\n");
    return ProcessExit();
}

int KernelWait(UserContext *uctxt){
    TracePrintf(1,"Enter KernelWait\n");
    pcb_t *proc = TurnNodeToPCB(currProc);
    int *status_ptr = (int *) uctxt->regs[0];

    int rc = InputSanityCheck(status_ptr);
    if (rc){
        TracePrintf(1, "Error!The status_ptr address:%d in KernelWait is not valid!\n", uctxt->regs[0]);
        return ERROR;
    }

    if (isemptylist(proc->children) && isemptylist(proc->terminatedchild)){
        return ERROR;
    }

    //If there is not child terminated, block itself
    if (isemptylist(proc->terminatedchild)){
        enblockqueue(currProc,blockqueue);
        switchproc();        
    }

    //"Reap" the terminated child and return its pid
    if (!isemptylist(proc->terminatedchild)){
        lstnode* remove = remove_head(proc->terminatedchild);
        pcb_t* removeproc = TurnNodeToPCB(remove);
        *status_ptr = removeproc->exitstatus;
        free(remove);
        return removeproc->pid;
    }

    return ERROR;
}



int KernelGetPid(){
    return TurnNodeToPCB(currProc)->pid;
}

//Capture TRAP_CLOCK
void TrapClock(UserContext *uctxt){
    TracePrintf(2, "TrapClock called\n");

    TracePrintf(2, "freeframe_list size: %d\n", freeframe_list->size);

    //Dealing with syscall--Delay
    if (!isemptylist(waitingqueue)){
        lstnode *traverse = waitingqueue->head->next;
        while(traverse != NULL && traverse->id != -1) {               
            pcb_t* proc = TurnNodeToPCB(traverse);
            if(proc->clock > 0){
                proc->clock--;
                TracePrintf(3, "proc->clock:%d\n", proc->clock);
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

    return;
}

int KernelDelay(UserContext *uctxt){
    TracePrintf(2, "Enter KernelDelay\n");

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
        TracePrintf(3, "pid = %d\n", TurnNodeToPCB(currProc)->pid);
        enwaitingqueue(currProc,waitingqueue);
        proc->clock = clock_ticks;

        switchproc();
    }
    TracePrintf(2, "Exit KernelDelay\n");
}

/*
   SYSCALL END
 */

int ProcessExit(){
    pcb_t* currPcb = TurnNodeToPCB(currProc);

    // If the initProcess Exit, halt the program
    if(currPcb->pid == 2) {
        Halt();
        return ERROR;
    }

    terminateProcess(currProc);
    TracePrintf(1,"Error! A process should never return from KernelExit\n");
    return ERROR;
}


int CheckAvailableFrame(lstnode *cur_p){
    int i, cnt = 0;
    pcb_t *pcb = TurnNodeToPCB(cur_p);  

    for (i=0; i < MAX_PT_LEN; i++){
        if (VALID == pcb->usrPtb[i].valid){
            cnt++;
        }
    }

    if (freeframe_list->size <= cnt){
        TracePrintf(1, "Error! There is no enough free frames!\n");
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

            //Temporary make it writable to copy things in the memory
            childPtb[i].prot = PROT_WRITE;

            //allocate a free frame to it
            lstnode *first = remove_head(freeframe_list);
            if (NULL == first) TracePrintf(1, "Remove_head in CopyUserProcess failed!");
            childPtb[i].pfn = first->id;

            //Use SAFETY_MARGIN_PAGE as buffer to copy
            g_pageTableR0[SAFETY_MARGIN_PAGE].pfn = childPtb[i].pfn;

            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

            //Calculate Destination and Source Addresses
            unsigned int desAddr = SAFETY_MARGIN_PAGE << PAGESHIFT;
            unsigned int srcAddr = (i << PAGESHIFT) + VMEM_1_BASE;

            memcpy((void *)desAddr, (void *)srcAddr, PAGESIZE);

            //Restore the TRUE Protection!
            childPtb[i].prot = parentPtb[i].prot;
        }
    }

    //Restore the buffer.
    g_pageTableR0[SAFETY_MARGIN_PAGE].valid = INVALID;
    g_pageTableR0[SAFETY_MARGIN_PAGE].prot = PROT_NONE;
    g_pageTableR0[SAFETY_MARGIN_PAGE].pfn = UNALLOCATED;

    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

    return;
}

int switchproc()
{
    TracePrintf(2,"Enter switchproc.\n");
    int rc = SUCCESS;

    lstnode *switchIn = firstnode(readyqueue);

    if (TERMINATED ==  TurnNodeToPCB(currProc)->procState){
        rc = KernelContextSwitch(MyTerminateKCS, (void *) currProc, (void *) switchIn);
        if (rc) TracePrintf(1,"Error! MyTerminateKCS in switchproc failed!\n");  

    } else if (!isemptylist(readyqueue)){
        TracePrintf(3, "currProc->id%d\n", TurnNodeToPCB(currProc)->pid );
        TracePrintf(3, "switchIn->id%d\n" ,TurnNodeToPCB(switchIn)->pid);
        rc = KernelContextSwitch(MyTrueKCS, (void *) currProc, (void *) switchIn);
        if (rc) TracePrintf(1,"Error! MyTrueKCS in switchproc failed!\n");
    }
    else{
        rc =  ERROR;
    }	

    return rc;
}

int switchnext()
{
    switchproc();
}

void terminateProcess(lstnode *procnode){
    TracePrintf(2, "Enter terminateProcess!\n");

    TracePrintf(2, "OMG! Pid:%d is TERMINATED here!\n", currProc->id);

    int i, rc;
    pcb_t* proc = TurnNodeToPCB(procnode);
    proc->procState = TERMINATED;

    switchproc();
    return;
}

int enreadyqueue(lstnode* procnode,dblist* queue)
{	
    TracePrintf(3, "Enter enreadyqueue\n");    
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
    TracePrintf(3, "Enter enwaitingqueue\n");    
    pcb_t* proc = TurnNodeToPCB(procnode);

    if (proc == NULL){
        return ERROR;
    }
    proc->procState = WAITING;
    insert_tail(procnode, queue);

    TracePrintf(3, "Exit enwaitingqueue\n"); 
    return 0;
}

lstnode* dewaitingqueue(lstnode* waitingnode,dblist* queue)
{
    TracePrintf(3,"Enter dewaitingqueue\n");
    TracePrintf(3,"Exit dewaitingqueue\n");   	
    return remove_node(TurnNodeToPCB(waitingnode)->pid,queue);
}

int enblockqueue(lstnode* procnode,dblist* queue)
{
    TracePrintf(3, "Enter enblockqueue\n");    
    pcb_t* proc = TurnNodeToPCB(procnode);

    if (proc == NULL){
        return ERROR;
    }
    proc->procState = WAITING;
    insert_tail(procnode, queue);

    TracePrintf(3, "Exit enblockqueue\n"); 
    return 0;
}

lstnode* deblockqueue(lstnode* waitingnode,dblist* queue)
{
    TracePrintf(3,"Enter deblockqueue\n");
    TracePrintf(3,"Exit deblockqueue\n");     
    return remove_node(TurnNodeToPCB(waitingnode)->pid,queue);
}

int enreaderwaitingqueue(lstnode* procnode,dblist* queue)
{
    TracePrintf(3, "Enter enreaderwaitingqueue\n");    
    pcb_t* proc = TurnNodeToPCB(procnode);

    if (proc == NULL){
        return ERROR;
    }
    proc->procState = WAITING;
    insert_tail(procnode, queue);

    TracePrintf(3, "Exit enreaderwaitingqueue\n"); 
    return 0;
}

lstnode* dereaderwaitingqueue(dblist* queue)
{
    TracePrintf(3,"Enter dereaderwaitingqueue\n");
    TracePrintf(3,"Exit dereaderwaitingqueue\n");     
    return remove_head(queue);
}

int enwriterwaitingqueue(lstnode* procnode,dblist* queue)
{
    TracePrintf(3, "Enter enwriterwaitingqueue\n");    
    // pcb_t* proc = TurnNodeToPCB(procnode);

    // if (proc == NULL){
    //     return ERROR;
    // }
    // proc->procState = WAITING;
    insert_tail(procnode, queue);

    TracePrintf(3, "Exit enwriterwaitingqueue\n"); 
    return SUCCESS;
}

lstnode* dewriterwaitingqueue(dblist* queue)
{
    TracePrintf(3,"Enter dewriterwaitingqueue\n");
    TracePrintf(3,"Exit dewriterwaitingqueue\n");     
    return remove_head(queue);
}


int enwaitlockqueue(lstnode* procnode,dblist* queue)
{
    TracePrintf(3, "Enter enwaitlockqueue\n");    
    pcb_t* proc = TurnNodeToPCB(procnode);

    if (proc == NULL){
        return ERROR;
    }
    proc->procState = WAITING;
    insert_tail(procnode, queue);

    TracePrintf(3, "Exit enwaitlockqueue\n"); 
    return 0;
}

lstnode* dewaitlockqueue(dblist* queue)
{
    TracePrintf(3,"Enter dewaitlockqueue\n");
    TracePrintf(3,"Exit dewaitlockqueue\n");     
    return remove_head(queue);
}

int encvarqueue(lstnode* procnode,dblist* queue)
{
    TracePrintf(3, "Enter enwaitingqueue\n");    
    pcb_t* proc = TurnNodeToPCB(procnode);

    if (proc == NULL){
        return ERROR;
    }
    proc->procState = WAITING;
    insert_tail(procnode, queue);

    TracePrintf(1, "Exit enwaitingqueue\n"); 
    return 0;
}

lstnode* decvarqueue(dblist* queue)
{
    TracePrintf(3,"Enter dewaitingqueue\n");
    TracePrintf(3,"Exit dewaitingqueue\n");     
    return remove_head(queue);
}

int enwaitcvarqueue(lstnode* procnode,dblist* queue)
{
    TracePrintf(3, "Enter enwaitcvarqueue\n");    
    pcb_t* proc = TurnNodeToPCB(procnode);

    if (proc == NULL){
        return ERROR;
    }
    proc->procState = WAITING;
    insert_tail(procnode, queue);

    TracePrintf(3, "Exit enwaitcvarqueue\n"); 
    return 0;
}

lstnode* dewaitcvarqueue(dblist* queue)
{
    TracePrintf(3,"Enter dewaitcvarqueue\n");
    TracePrintf(3,"Exit dewaitcvarqueue\n");     
    return remove_head(queue);
}
int enwaitsemqueue(lstnode* procnode,dblist* queue)
{
    TracePrintf(3, "Enter enwaitcvarqueue\n");    
    pcb_t* proc = TurnNodeToPCB(procnode);

    if (proc == NULL){
        return ERROR;
    }
    proc->procState = WAITING;
    insert_tail(procnode, queue);

    TracePrintf(3, "Exit enwaitcvarqueue\n"); 
    return 0;
}

lstnode* dewaitsemqueue(dblist* queue)
{
    TracePrintf(3,"Enter dewaitcvarqueue\n");
    TracePrintf(3,"Exit dewaitcvarqueue\n"); 
    return remove_head(queue);

}   


int enbufferqueue(lstnode* procnode,dblist* queue)
{
    TracePrintf(3, "Enter enbufferqueue\n");    
    insert_tail(procnode, queue);
    TracePrintf(3, "Exit enbufferqueue\n"); 
    return SUCCESS;
}

lstnode* debufferqueue(dblist* queue)
{
    TracePrintf(3,"Enter debufferqueue\n");
    TracePrintf(3,"Exit debufferqueue\n"); 
    return remove_head(queue);
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
