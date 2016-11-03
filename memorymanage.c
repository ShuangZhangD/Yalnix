#include "memorymanage.h"
#include "hardware.h"
#include "listcontrol.h"

extern dblist* freeframe_list;
extern lstnode* currProc;


int kernelbrk(UserContext *uctxt){
    int i,rc;
    pcb_t *proc = TurnNodeToPCB(currProc);

    unsigned int newBrk = (unsigned int) uctxt->regs[0];
    int oldBrkPage = proc->brk_page;
    int stacklimitpage = proc->stack_limit_page;
    int dataPage = proc->data_page;
    int newBrkPage = newBrk >> PAGESHIFT;

    TracePrintf(1,"oldBrkPage:%d, stacklimitpage:%d, dataPage:%d, newBrkPage:%d\n", oldBrkPage, stacklimitpage, dataPage, newBrkPage);

    if(newBrkPage >= stacklimitpage - 1){
        return ERROR;
    } else if (newBrkPage > oldBrkPage){
        
        writepagetable(proc->usrPtb, oldBrkPage, newBrkPage, VALID, (PROT_READ | PROT_WRITE));
        //Flush Tlb!
        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

    } else if (newBrkPage < oldBrkPage){
        
        if (newBrkPage < dataPage) return ERROR;
        //Remap  Add this frame back to free frame tracker
        ummap(proc->usrPtb, newBrkPage, oldBrkPage, INVALID, PROT_NONE);
        
        //FLUSH!!!
        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

    }else {
        //Do nothing
    }

    //Let addr be the new kernel break
    proc->brk_page = newBrkPage;

    return SUCCESS;
}

//Capture TRAP_MEMORY
void TrapMemory(UserContext *uctxt){
    TracePrintf(1,"TrapMemory\n");
    pcb_t *proc = TurnNodeToPCB(currProc);
    
    TracePrintf(1, "uctxt->addr:%p\n", uctxt->addr);
    
    int rc;
    int trapCode = uctxt->code;
    unsigned int newStackPage = ((unsigned int) uctxt->addr) >> PAGESHIFT;

    switch(trapCode){
        case (YALNIX_MAPERR):
            if (newStackPage > proc->stack_limit_page){
                terminateProcess(currProc);
                return;
            }

            if (newStackPage < proc->brk_page){
                terminateProcess(currProc);
                return;
            }

            rc = GrowUserStack(currProc,newStackPage);
            if (rc){
                terminateProcess(currProc);
                return;
            }
            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
            break;
        case (YALNIX_ACCERR):
            terminateProcess(currProc);
            break;
        default:
            terminateProcess(currProc);
            break;
    }

    return;

    /*
       IF  [current break of heap] < uctxt->addr < [allocated memory for the stack](uctxt->ebp)
       keep going 
       ELSE 
       not Allocate    

       IF at least on page  
       Allocate memory for stack
       ELSE
       Abort current process

       Rearrange queue 
     */
}


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