#include "memorymanage.h"
#include "hardware.h"
#include "listcontrol.h"

extern dblist* freeframe_list;

//TODO return code
void writepagetable(pte_t *pagetable, int startPage, int endPage, int valid, int prot){
 	int i;
 	if (!isemptylist(freeframe_list)){
		for (i=startPage; i<=endPage; i++){
			pagetable[i].valid = valid;
			pagetable[i].prot = prot;
			
	        lstnode *first = remove_head(freeframe_list);
	        pagetable[i].pfn = first->id;
		}
	} else {	
		//TODO
	}
	return;
}

void ummap(pte_t *pagetable, int startPage, int endPage, int valid, int prot){
	int i;
	for (i=startPage; i<=endPage; i++){

		int pageNumber = pagetable[i].pfn;
		lstnode *frame = nodeinit(pageNumber);
        insert_tail(frame,freeframe_list);

		pagetable[i].valid = valid;
		pagetable[i].prot = prot;
		pagetable[i].pfn = UNALLOCATED;
	}

	return;
}

void emptyregion1pagetable(pcb_t *proc){
	int i;
	for (i=0; i<MAX_PT_LEN;i++){
		if (1 == proc->usrPtb[i].valid){
			proc->usrPtb[i].valid = 0;
			int usedFrame = proc->usrPtb[i].pfn;

			lstnode *frame = nodeinit(usedFrame);
    	    insert_tail(frame,freeframe_list);	
		}	
	}

	return;
}

int GrowUserStack(lstnode *procnode, int addrPage){
	pcb_t *proc = TurnNodeToPCB(procnode);

	int oldStackPage = proc->stack_limit_page;  
	int newStackPage = addrPage;
	int i;
	
	//Check How Many FreeFrame do we have, plus 1 for safety margin
	if (freeframe_list->size < newStackPage-oldStackPage+1){
		return -1;
	}

	writepagetable(proc->usrPtb, oldStackPage, newStackPage, VALID, (PROT_READ | PROT_WRITE));
	proc->stack_limit_page = newStackPage;

	return 0;
}