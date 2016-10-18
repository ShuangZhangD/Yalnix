#include "kernel.h"
#include "yalnix.h"
#include "loadprogram.h"
#include "listcontrol.h"

//Global Variables
int m_enableVM = 0; //A flag to check whether Virtual Memory is enabled(1:enabled, 0:not enabled)
int g_pid = 1;


dblist* freeframe_list;


int kernelfork(UserContext *uctxt){
    return ERROR;
}

int kernelexec(UserContext *uctxt){
    return ERROR;
}

int kernelexit(UserContext *uctxt){
    return ERROR;
}

int kernelwait(UserContext *uctxt){
    return ERROR;
}

int kernelgetpid(UserContext *uctxt){
    return ERROR;
}

int kernelbrk(UserContext *uctxt){
    return ERROR;            
}

int kerneldelay(UserContext *uctxt){
    return ERROR;
}

int kernelregister(UserContext *uctxt){
    return ERROR;
}

int kernelsend(UserContext *uctxt){
    return ERROR;
}

int kernelreceive(UserContext *uctxt){
    return ERROR;
}

int kernelreceivespecific(UserContext *uctxt){
    return ERROR;
}

int kernelreply(UserContext *uctxt){
    return ERROR;
}

int kernelforward(UserContext *uctxt){
    return ERROR;
}

int kernelcopyfrom(UserContext *uctxt){
    return ERROR;
}

int kernelcopyto(UserContext *uctxt){
    return ERROR;
}

int kernelreadsector(UserContext *uctxt){
    return ERROR;
}

int kernelwritesector(UserContext *uctxt){
    return ERROR;
}

int kernelreclaim(int id){
    //Destroy Cvar, lock, pipe identified by Id 
    return ERROR;
}

/*
    Kernel Initiailization functions
*/
void SetKernelData(void *_KernelDataStart, void *_KernelDataEnd){
    m_kernel_brk = (unsigned int) _KernelDataEnd;
    m_kernel_data_start = (unsigned int) _KernelDataStart;

    return;
}

void InitUserPageTable (pcb_t *proc){
    int i;
    
    //Mark User Page table as Invalid;
    for (i = 0; i < MAX_PT_LEN; i++){
        proc->usrPtb[i].valid = 0;
    }

    return;
}


void InitKernelPageTable(pcb_t *proc) {
    
    unsigned int kDataEdPage = m_kernel_brk >> PAGESHIFT; 
    unsigned int kDataStPage = m_kernel_data_start >> PAGESHIFT;
    unsigned int kStackStPage = KERNEL_STACK_LIMIT >> PAGESHIFT;
    unsigned int kStackEdPage = KERNEL_STACK_BASE >> PAGESHIFT;
    int stackInx ;
    int numOfStack = kStackEdPage - kStackStPage + 1;
    int i;
    
    //Protect Kernel Text, Data and Heap
    for (i=0; i < kDataEdPage; i++){
        if (i < kDataStPage){
            //Protect Kernel Text
            g_pageTableR0[i].valid = 1;
            g_pageTableR0[i].prot = (PROT_READ | PROT_EXEC);
            g_pageTableR0[i].pfn = i;
            lstnode *frame;
            frame->id = i;
            remove_node(frame, freeframe_list);
        } else {
            //Protect Kernel Data & Heap
            g_pageTableR0[i].valid = 1;
            g_pageTableR0[i].prot = (PROT_READ | PROT_WRITE);
            g_pageTableR0[i].pfn = i;
            lstnode *frame;
            frame->id = i;
            remove_node(frame, freeframe_list);
        }
    }
    
    proc->krnlStackPtb = (pte_t *) calloc(numOfStack ,sizeof(pte_t));
    proc->krnlStackPtbSize = numOfStack;

    //Protect Kernel Stack
    for (i=kStackStPage, stackInx = 0; i< kStackEdPage; i++, stackInx++){
        g_pageTableR0[i].valid = 1;
        g_pageTableR0[i].prot = (PROT_READ | PROT_WRITE);
        g_pageTableR0[i].pfn = i;
        lstnode *frame;
        frame->id = i;
        remove_node(frame, freeframe_list);
        //Let a userprocess have its own kernel stack
        if (stackInx < numOfStack){
            proc->krnlStackPtb[stackInx] = g_pageTableR0[i];
        } else {
            //TODO print ErrorMsg
        }
    }
    
    return;

}

pcb_t *InitPcb(UserContext *uctxt){
    //Initialize Queue TODO Jason Please finish this! At least finish running queue!
    
    //Initialize Process
    pcb_t *proc = (pcb_t *) malloc (sizeof(pcb_t));
    proc->procState = RUNNING;
    proc->pid = g_pid++;
    proc->uctxt = uctxt;
    
    return proc;
}

void KernelStart(char *cnd_args[],unsigned int pmem_size, UserContext *uctxt){
    int i;
    //Initialize interrupt vector table and REG_VECTOR_BASE
    InitInterruptTable();
    
    //Initialize New Process
    pcb_t *idleProc = InitPcb(uctxt);
    
    //Build a structure to track free frame
    int numOfFrames = (pmem_size / PAGESIZE);
    dblist* listinit(freeframe_list);
    lstnode *frame;
    for(i = 0;i<numOfFrames;i++)
    {
        frame->id = i;
        insert_tail(frame,freeframe_list);
    }
    //keep track of free frame;
    //TODO Jason Please finish this tracker! Thanks!
    
    //Build initial page table for Region 0
    InitKernelPageTable(idleProc);
    WriteRegister(REG_PTBR0, (unsigned int) &g_pageTableR0);
    WriteRegister(REG_PTLR0, (unsigned int) MAX_PT_LEN);
    
    //Build initial page table for Region 1
    InitUserPageTable(idleProc);
    WriteRegister(REG_PTBR1, (unsigned int) &(idleProc->usrPtb));
    WriteRegister(REG_PTLR1, (unsigned int) MAX_PT_LEN);

    // Enable virtual memory
    WriteRegister(REG_VM_ENABLE,(unsigned int) 1);
    m_enableVM = 1;
    
    
    //====Cook DoIdle()====
    idleProc->usrPtb[0].valid = 1; 
    idleProc->usrPtb[0].prot = (PROT_WRITE | PROT_READ);
    idleProc->usrPtb[0].pfn = 0x001;//TODO Allocate a free frame and give it a number

    //Allocate One page to it
    idleProc->uctxt->sp = (void *) (VMEM_1_LIMIT - PAGESIZE - INITIAL_STACK_FRAME_SIZE);
    
    //Get the function pointer of DoIdle
    void (*idlePtr)(void) = &DoIdle;
    idleProc->uctxt->pc = idlePtr;
    
    //====================

    //Create first process  and load initial program to it
    // loadprogram(char *name, char *args[], proc);

    return;
}

//Do Idle Process
void DoIdle (void){
    while(1){
        TracePrintf(1, "Doodle\n");
        Pause();
    }
    return;
}

int checkPageStatus(unsigned int addr){
    unsigned int i, pageAddr = (addr >> PAGESHIFT),rc = 0;

    //make sure addresses from [addr] to [VMEM_BASE] are valid
    for (i = (VMEM_BASE >> PAGESHIFT); i < pageAddr; i++){
        if (0 == g_pageTableR0[i].valid) return -1;
    }

    //check no virtual memory in Region 0 out of range are valid
    for (i = pageAddr; i < MAX_PT_LEN; i++){
        if (1 == g_pageTableR0[i].valid) return -1;
    }

    return 0;
}


int SetKernelBrk(void *addr){
    //returns to the kernel lib
    unsigned int newBrk = (unsigned int) addr;


    int rc;
    if (m_enableVM){

        rc = checkPageStatus(newBrk);
        if (rc) return -1;
        
        int newBrkPage = newBrk >> PAGESHIFT;
        int oldBrkPage = m_kernel_brk >> PAGESHIFT;
        int i;
        if (newBrk > m_kernel_brk){

            //TODO Jason Please finish this function(), I give you a sketch here.
            if (isemptylist(freeframe_list)){
                g_pageTableR0[i].valid = 1;
                g_pageTableR0[i].prot = (PROT_READ | PROT_WRITE);
                lstnode *first = firstnode(freeframe_list);
                g_pageTableR0[i].pfn = first->id;//TODO Physical Frame Number; 
            }
            //FLUSH!!!
                
        }else if (newBrk < m_kernel_brk){

            //TODO Jason Please finish this function(), I give you a sketch here.

            g_pageTableR0[oldBrkPage].valid = 0;
            lstnode *frame;
            for(i = newBrkPage;i < oldBrkPage;i++){
            frame->id = i;
            insert_tail(frame,freeframe_list);
            }
            //Add this frame back to free frame tracker

            //FLUSH!!!
            if (rc) return -1;
        }
        //Let addr be the new kernel break
    
    }
    m_kernel_brk = newBrk;
    return 0;
}

/*
    Context Switch
*/
// when someone calls KernelContextSwitch, it might come to here.
KernelContext *MyKCS(KernelContext *kc_in,void *curr_pcb_p,void *next_pcb_p){
    pcb_t *cur = (pcb_t *) curr_pcb_p;
    pcb_t *next = (pcb_t *) next_pcb_p;

    //Copy the kernel context to current process's pcb
    cur->kctxt = kc_in;

    //Remember to change page table entries for kernel stack

    //Return a pointer to a kernel context it had earlier saved
    return next->kctxt;

}

