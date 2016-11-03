#include "processmanage.h"
#include "selfdefinedstructure.h"

extern dblist* freeframe_list;
extern lstnode* currProc;

int switchproc()
{
        TracePrintf(2,"Enter switchproc.\n");
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
    int i;
    pcb_t* proc = (pcb_t*)procnode->content;
    proc->procState = TERMINATED;

    for (i = 0; i < MAX_PT_LEN; i++){
        if (proc->usrPtb[i].valid){
            proc->usrPtb[i].valid = 0;
	    	remove_node(i , freeframe_list);   
	    }
    }

    //TODO terminatedQueue(proc->pid);
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