#include "memorymanage.h"
#include "hardware.h"
#include "listcontrol.h"

extern dblist* freeframe_list;

//TODO return code
void writepagetable(pte_t *pagetable, int startPage, int endPage, int valid, int prot){
 	int i;
	for (i=startPage; i<=endPage; i++){
		pagetable[i].valid = 1;
		pagetable[i].prot = prot;
		//TODO Find Available node
        remove_node(i, freeframe_list);
	}

	return;
}


void emptyregion1pagetable(pcb_t *proc){
	int i;
	for (i=0; i<MAX_PT_LEN;i++){
		if (1 == proc->usrPtb[i].valid){
			proc->usrPtb[i].valid = 0;
			int pageFrameNum = proc->usrPtb[i].pfn;


			//TODO Deallocate
		}
	}
}