#include "pcb.h"

void terminateProcess(pcb_t *proc){
    int i;
    proc->procState = TERMINATED;

    for (i = 0; i < MAX_PT_LEN; i++){
        if (proc->usrPtb[i].valid){
            proc->usrPtb[i].valid = 0;
            //TODO ReleaseFrame
        }
    }

    //TODO terminatedQueue(proc->pid);

    //TODO Delete process or relocate process pool

}

int GrowUserStack(pcb_t *proc, unsigned int addr){
	int oldStackPage = (proc->sp >> PAGESHIFT);  
	int newStackPage = (addr >> PAGESHIFT);
	int i, rc;
	
	//TODO check How Many FreeFrame do we have, plus 1 for safety margin
	rc = 0;//checkAvailFrame(newStackPage-oldStackPage+1);
	if (rc) return -1;

	for (i = oldStackPage + 1; i <= newStackPage; i++){
		proc->usrPtb[i].valid = 1;
		proc->usrPtb[i].prot = (PROT_READ | PROT_WRITE);
		proc->usrPtb[i].pfn = 0x001; //TODO JASON's Method
	}

	proc->sp = addr;

	return 0;
}