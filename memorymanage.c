#include "memorymanage.h"
#include "hardware.h"
#include "listcontrol.h"

extern dblist* freeframe_list;

//TODO return code
void writepagetable(pte_t *pagetable, int startPage, int endPage, int valid, int prot){
 	int i;
	for (i=startPage; i<=endPage; i++){
		pagetable[i].valid = valid;
		pagetable[i].prot = prot;
		
		// TracePrintf(1, "====================\n");
		// traverselist(freeframe_list);

        lstnode *first = remove_head(freeframe_list);
        pagetable[i].pfn = first->id;
        // TracePrintf(1, "pfn=:%d\n", first->id);
        // TracePrintf(1, "====================\n"); 
	}

	return;
}


void emptyregion1pagetable(pcb_t *proc){
	int i;
	for (i=0; i<MAX_PT_LEN;i++){
		if (1 == proc->usrPtb[i].valid){
			proc->usrPtb[i].valid = 0;
			int usedFrame = proc->usrPtb[i].pfn;

            lstnode *freeFrame = remove_node(usedFrame, freeframe_list);
            insert_tail(freeFrame, freeframe_list);
		}
	}

	return;
}

int GrowUserStack(lstnode *procnode, unsigned int addr){
	pcb_t* proc = (pcb_t*)procnode->content;
	int oldStackPage = (proc->sp >> PAGESHIFT);  
	int newStackPage = (addr >> PAGESHIFT);
	int i;
	
	//TODO check How Many FreeFrame do we have, plus 1 for safety margin
	if (freeframe_list->size < newStackPage-oldStackPage+1){
		return -1;
	}

	for (i = oldStackPage + 1; i <= newStackPage; i++){
		proc->usrPtb[i].valid = 1;
		proc->usrPtb[i].prot = (PROT_READ | PROT_WRITE);
		proc->usrPtb[i].pfn = remove_head(freeframe_list)->id;
	}

	proc->sp = addr;

	return 0;
}