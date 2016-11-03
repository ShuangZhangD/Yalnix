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
    lstnode* child; //= initProc(currProc); //TODO Replace new initproc
    pcb_t* childproc = (pcb_t*) child->content;
    pcb_t* parentproc = (pcb_t*)currProc->content;

    childproc->parent = currProc;

    int i, stackInx;

    //Initialize Process
    pcb_t *proc = (pcb_t *) malloc (sizeof(pcb_t));
    proc->pid = g_pid++;

    proc->krnlStackPtb = (pte_t *) calloc(g_pageNumOfStack ,sizeof(pte_t));
    proc->krnlStackPtbSize = g_pageNumOfStack;

    proc->usrPtb = InitUserPageTable();

    // pte_t *usrPtb = (pte_t *) malloc(sizeof(pte_t) * MAX_PT_LEN);

    //Mark User Page table as Invalid;
    for (i = 0; i < MAX_PT_LEN; i++){
        childproc->usrPtb[i].valid = 0;
        childproc->usrPtb[i].prot = PROT_NONE;
        childproc->usrPtb[i].pfn = 0; //TODO make sure it is right
    }


    memcpy();

    proc->krnlStackPtb = (pte_t *) calloc(g_pageNumOfStack ,sizeof(pte_t));
    proc->krnlStackPtbSize = g_pageNumOfStack;

    //Let a userprocess have its own kernel stack
    for (i=g_kStackStPage, stackInx = 0; i<=g_kStackEdPage; i++, stackInx++){
        proc->krnlStackPtb[stackInx] = g_pageTableR0[i];
    }


    // return TurnPCBToNode(proc);
    return 0; // TODO

    insert_tail(child,proc->children);

    enreadyqueue(currProc,waitingqueue);

    if(currProc == child)
    {
        return 0;
    }
    else{
        return childproc->pid;
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
    SYSCALL
*/


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

    // for (i = 0; i < MAX_PT_LEN; i++){
    //     if (proc->usrPtb[i].valid){
    //         proc->usrPtb[i].valid = 0;
    //         int frameId = proc->usrPtb[i].pfn;
	   //  	lstnode* node = remove_node(frameId , freeframe_list);   
    //         if (NULL == node){
    //             TracePrintf(1, "terminateProcess failed!\n");
    //             return;
    //         }
	   //  }
    // }

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