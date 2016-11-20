#include "memorymanage.h"
#include "hardware.h"
#include "listcontrol.h"

extern dblist* freeframe_list;
extern lstnode* currProc;

/*=========================================Trap & Syscalls =========================================*/

int KernelBrk(UserContext *uctxt){
    TracePrintf(2, "Enter KernelBrk\n");

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

    /*
        if new break >= stack limit => Error
        if new break < old break => Unmap old frames
        if new break > old break => map new frames
        if new break == old break => do nothing
    */
    if(newBrkPage >= stacklimitpage - 1){
        return ERROR;
    } else if (newBrkPage > oldBrkPage){
        TracePrintf(3, "Grow New Brk in KernelBrk\n");        
        
        rc = WritePageTable(proc->usrPtb, oldBrkPage, newBrkPage, VALID, (PROT_READ | PROT_WRITE));
        if (rc){
            TracePrintf(1, "Error! WritePageTable Failed in KernelBrk\n");
            return ERROR;
        }
        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1); //Flush Tlb!
    } else if (newBrkPage < oldBrkPage){
        TracePrintf(3, "Lower Brk in KernelBrk\n");
        if (newBrkPage <= dataPage) return ERROR;

        rc = Unmap(proc->usrPtb, newBrkPage, oldBrkPage, INVALID, PROT_NONE);
        if (rc){
            TracePrintf(1, "Error! Unmap Failed in KernelBrk\n");
            return ERROR;
        }
        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1); //Flush Tlb!

    }else {
        //Do nothing
    }

    //Let addr be the new kernel break
    proc->brk_page = newBrkPage;  

    TracePrintf(2, "Exit KernelBrk\n");
    return SUCCESS;
}

//Capture TRAP_MEMORY
void TrapMemory(UserContext *uctxt){
    TracePrintf(2,"Enter TrapMemory\n");

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
                ProcessExit();
                return;
            }

            if (newStackPage < proc->brk_page){
                TracePrintf(1, "Error! The new Stack Address:%d is INVALID!\n",uctxt->addr);
                ProcessExit();
                return;
            }

            rc = GrowUserStack(currProc,newStackPage);
            TracePrintf(3, "The new Stack Address is: %d\n",uctxt->addr);
            if (rc){
                TracePrintf(1, "Error! GrowUserStack Failed!\n");
                ProcessExit();
                return;
            }
            break;
        case (YALNIX_ACCERR):
            TracePrintf(1, "Error! Touch Illegal Address!\n");
            ProcessExit();
            break;
        default:
            ProcessExit();
            break;
    }

    TracePrintf(2,"Exit TrapMemory\n");
    return;

    /*
       IF  [current break of heap] < uctxt->addr < [allocated memory for the stack](uctxt->ebp)
       keep going 
       ELSE 
       not Allocate    

       IF at least one page  
       Allocate memory for stack
       ELSE
       Abort current process
     */
}
/*=========================================Trap & Syscalls =========================================*/

int WritePageTable(pte_t *pagetable, int startPage, int endPage, int valid, int prot){
 	int i;
 	if (!isemptylist(freeframe_list)){
		for (i=startPage; i<=endPage; i++){
			pagetable[i].valid = valid;
			pagetable[i].prot = prot;
			
	        lstnode *first = remove_head(freeframe_list);
	        pagetable[i].pfn = first->id;
		}
	} else {	
        TracePrintf(1, "Error! There is no free frame to WritePageTable\n");
		return ERROR;
	}
	return SUCCESS;
}

int Unmap(pte_t *pagetable, int startPage, int endPage, int valid, int prot){
	int i;
	for (i=startPage; i<=endPage; i++){
        if (VALID == pagetable[i].valid){
		    
            int pageNumber = pagetable[i].pfn;
		    
            lstnode *frame = nodeinit(pageNumber);
            if (NULL == frame){
                TracePrintf(1, "Unmap Failed!\n");
                return ERROR;
            }
            insert_tail(frame,freeframe_list);
        }
		pagetable[i].valid = valid;
		pagetable[i].prot = prot;
		pagetable[i].pfn = UNALLOCATED;
	}

	return SUCCESS;
}

int EmptyRegion1PageTable(pcb_t *proc){
    return Unmap(proc->usrPtb, 0, MAX_PT_LEN-1, INVALID, PROT_NONE);
}

int GrowUserStack(lstnode *procnode, int addrPage){
	pcb_t *proc = TurnNodeToPCB(procnode);

	int oldStackPage = proc->stack_limit_page;  
	int newStackPage = addrPage;
	int i;
	
	//Check How Many FreeFrame do we have, plus 1 for safety margin
	if (freeframe_list->size < newStackPage-oldStackPage+1){
		return ERROR;
	}

	WritePageTable(proc->usrPtb, oldStackPage, newStackPage, VALID, (PROT_READ | PROT_WRITE));
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1); //FLUSH!
	proc->stack_limit_page = newStackPage;

	return SUCCESS;
}

void* MallocCheck(int size){
    void* mm = (void* ) malloc(size);
    bzero(mm, size);

    return mm;
}

int InputSanityCheck(int *addr){
    int ad = (int) addr;
    int page = (ad - VMEM_1_BASE) >> PAGESHIFT;


    if (ad < VMEM_1_BASE) return ERROR;

    if (page > MAX_PT_LEN) return ERROR;

    // if (TurnNodeToPCB(currProc)->data_page < page && page <  TurnNodeToPCB(currProc)->stack_limit_page){
    //     return ERROR;
    // } 

    return SUCCESS;
}
