#include "pcb.h"
#include "selfdefinedstructure.h"

extern dblist* freeframe_list;
extern lstnode* currProc;

int switchproc()
{
        
        int rc;
        if (!isemptylist(readyqueue))
        {
            rc = KernelContextSwitch(MyTrueKCS, (void *) currProc, (void *) firstnode(readyqueue));
            currProc = dereadyqueue(readyqueue);
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

int enreadyqueue(lstnode* procnode,dblist* readyqueue)
{	
    TracePrintf(1, "Enter enreadyqueue\n");    
	pcb_t* proc = TurnNodeToPCB(procnode);;

	if (proc == NULL){
		return ERROR;
	}
	proc->procState = READY;
	insert_tail(procnode,readyqueue);

	return 0;
}

void* dereadyqueue(dblist* readyqueue)
{
	return (void*)remove_head(readyqueue)->content;
}

int enwaitingqueue(lstnode* procnode,dblist* waitingqueue)
{
	pcb_t* proc = (pcb_t*)procnode->content;

	if (proc == NULL){
		return ERROR;
	}
	proc->procState = WAITING;
	insert_tail(procnode,readyqueue);
	return 0;
}

void* dewaitingqueue(lstnode* waitingnode,dblist* waitingqueue)
{
	
	return (void*)remove_node(((pcb_t*)waitingnode->content)->pid,waitingqueue)->content;
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