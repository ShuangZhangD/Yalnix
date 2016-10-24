#include "pcb.h"
#include "selfdefinedstructure.h"

extern dblist* freeframe_list;


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
	pcb_t* proc = (pcb_t*)procnode->content;

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

int GrowUserStack(lstnode *procnode, unsigned int addr){
	pcb_t* proc = (pcb_t*)procnode->content;
	int oldStackPage = (proc->sp >> PAGESHIFT);  
	int newStackPage = (addr >> PAGESHIFT);
	int i, rc;
	
	//TODO check How Many FreeFrame do we have, plus 1 for safety margin
	rc = 0;//checkAvailFrame(newStackPage-oldStackPage+1);
	if (rc) return -1;

	for (i = oldStackPage + 1; i <= newStackPage; i++){
		proc->usrPtb[i].valid = 1;
		proc->usrPtb[i].prot = (PROT_READ | PROT_WRITE);
		proc->usrPtb[i].pfn = remove_head(freeframe_list)->id; //TODO JASON's Method
	}

	proc->sp = addr;

	return 0;
}