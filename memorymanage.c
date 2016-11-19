#include "memorymanage.h"
#include "hardware.h"
#include "listcontrol.h"

extern dblist* freeframe_list;
extern lstnode* currProc;


int kernelbrk(UserContext *uctxt){
    pcb_t *proc = TurnNodeToPCB(currProc);

    int i,rc;    
    unsigned int newBrk = (unsigned int) uctxt->regs[0];
    int oldBrkPage = proc->brk_page;
    int stacklimitpage = proc->stack_limit_page;
    int dataPage = proc->data_page;

    if (newBrk < VMEM_1_BASE){
        TracePrintf(1,"Error! The New Break Address:%d is lower than the address of region 1!\n", newBrk);
        return ERROR;
    }
    int newBrkPage = (newBrk - VMEM_1_BASE)>> PAGESHIFT;

    TracePrintf(3,"oldBrkPage:%d, stacklimitpage:%d, dataPage:%d, newBrkPage:%d\n", oldBrkPage, stacklimitpage, dataPage, newBrkPage);

    if(newBrkPage >= stacklimitpage - 1){
        return ERROR;
    } else if (newBrkPage > oldBrkPage){
        TracePrintf(2, "Grow New Brk in kernelbrk\n");        
        writepagetable(proc->usrPtb, oldBrkPage, newBrkPage, VALID, (PROT_READ | PROT_WRITE));
        //Flush Tlb!
        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
    } else if (newBrkPage < oldBrkPage){
        TracePrintf(2, "Lower Brk in kernelbrk\n");
        if (newBrkPage <= dataPage) return ERROR;
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
    TracePrintf(2,"TrapMemory\n");

    int rc;
    int trapCode = uctxt->code;
    unsigned int newStackPage = ((unsigned int) uctxt->addr - VMEM_1_BASE) >> PAGESHIFT;

    TracePrintf(3, "uctxt->addr:%p\n", uctxt->addr);

    pcb_t *proc = TurnNodeToPCB(currProc);
    proc->uctxt = *uctxt;
        
    switch(trapCode){
        case (YALNIX_MAPERR):
            if (newStackPage > proc->stack_limit_page){
                TracePrintf(1, "Error! The new Stack Address:%d is INVALID!\n",uctxt->addr);
                terminateProcess(currProc);
                return;
            }

            if (newStackPage < proc->brk_page){
                TracePrintf(1, "Error! The new Stack Address:%d is INVALID!\n",uctxt->addr);
                terminateProcess(currProc);
                return;
            }

            rc = GrowUserStack(currProc,newStackPage);
            TracePrintf(3, "The new Stack Address is: %d\n",uctxt->addr);
            if (rc){
                terminateProcess(currProc);
                return;
            }
            break;
        case (YALNIX_ACCERR):
            TracePrintf(1, "Error! Touch Illegal Address!\n");
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
        if (VALID == pagetable[i].valid){
		    int pageNumber = pagetable[i].pfn;
		    lstnode *frame = nodeinit(pageNumber);
            insert_tail(frame,freeframe_list);
        }
		pagetable[i].valid = valid;
		pagetable[i].prot = prot;
		pagetable[i].pfn = UNALLOCATED;
	}

	return;
}

void emptyregion1pagetable(pcb_t *proc){
    ummap(proc->usrPtb, 0, MAX_PT_LEN-1, INVALID, PROT_NONE);
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
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
	proc->stack_limit_page = newStackPage;

	return 0;
}

void* MallocCheck(int size){
    void* mm = (void* ) malloc(size);
    bzero(mm, size);

    return mm;
}

int InputSanityCheck(int *addr){
    int ad = (int) addr;
    int page = (ad - VMEM_1_BASE) >> PAGESHIFT;

    if (page > MAX_PT_LEN){
        return ERROR;
    }

    if (page <  TurnNodeToPCB(currProc)->stack_limit_page){
        return ERROR;
    } 

    return SUCCESS;
}

int IOSanityCheck(int *addr){
    int ad = (int) addr;
    int page = (ad - VMEM_1_BASE) >> PAGESHIFT;

    if (page > MAX_PT_LEN){
        return ERROR;
    }

    if (TurnNodeToPCB(currProc)->data_page < page && page <  TurnNodeToPCB(currProc)->stack_limit_page){
        return ERROR;
    } 

    return SUCCESS;


}