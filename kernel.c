#include"kernel.h"
#include"yalnix.h"
#include"hardware.h"
#include"loadprogram.h"

//Global Variables
int m_enableVM = 0; //A flag to check whether Virtual Memory is enabled(1:enabled, 0:not enabled)
int g_pid = 0;
unsigned int m_kernel_brk;
unsigned int m_kernel_data_start;

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
    
    unsigned int kDataEdPage = DOWN_TO_PAGE(m_kernel_brk); 
    unsigned int kDataStPage = DOWN_TO_PAGE(m_kernel_data_start);
    unsigned int kStackStPage = DOWN_TO_PAGE(KERNEL_STACK_LIMIT);
    unsigned int kStackEdPage = DOWN_TO_PAGE(KERNEL_STACK_BASE);
    int stackInx = 0;
    int numOfStack = kStackEdPage - kStackStPage + 1;
    int i;
    
    //Protect Kernel Text, Data and Heap
    for (i=0; i < kDataEdPage; i++){
        if (i < kDataStPage){
            //Protect Kernel Text
            g_pageTableR0[i].valid = 1;
            g_pageTableR0[i].prot = (PROT_READ | PROT_EXEC);
            g_pageTableR0[i].pfn = i;
        } else {
            //Protect Kernel Data & Heap
            g_pageTableR0[i].valid = 1;
            g_pageTableR0[i].prot = (PROT_READ | PROT_WRITE);
            g_pageTableR0[i].pfn = i;
        }
    }
    
    proc->krnlPtb = (pte_t *) calloc(numOfStack ,sizeof(pte_t));
    proc->krnlPtbSize = numOfStack;

    //Protect Kernel Stack
    for (i=kStackStPage, stackInx; i< kStackEdPage && stackInx < numOfStack; i++, stackInx++ ){
        g_pageTableR0[i].valid = 1;
        g_pageTableR0[i].prot = (PROT_READ | PROT_WRITE);
        g_pageTableR0[i].pfn = i;
        
        //Let a userprocess have its own kernel stack
        if (stackInx < numOfStack){
            proc->krnlPtb[stackInx] = g_pageTableR0[i];
        } else {
            //TODO print ErrorMsg
        }
    }
    
    return;

}

pcb_t *InitPcb(){
    //Initialize Queue;

    //Initialize Process

}

void KernelStart(char *cnd_args[],unsigned int pmem_size, UserContext *uctxt){

    //Initialize interrupt vector table and REG_VECTOR_BASE
    InitInterruptTable();
    
    //Initialize New Process
    pcb_t *idleProc = InitPcb();
    
    //Build a structure to track free frame
    g_freeFrame = listinit();

    int numOfFrames = (pmem_size / PAGESIZE);
    //keep track of free frame;
    //TODO
     
    //Build initial page table for Region 0
    InitKernelPageTable(idleProc);
    WriteRegister(REG_PTBR0, (unsigned int) &g_pageTableR0);
    WriteRegister(REG_PTLR0, (unsigned int) MAX_PT_LEN);
    
    //Build initial page table for Region 1
    InitUserPageTable(idleProc);
    WriteRegister(REG_PTBR1, (unsigned int) &(idleProc->usrPtb));
    WriteRegister(REG_PTLR1, (unsigned int) MAX_PT_LEN);

    // Enable virtual memory
    WriteRegister(REG_VM_ENABLE,1);
    m_enableVM = 1;
    
    //Cook DoIdle()
    idleProc->usrPtb[0].valid = 1; 
    idleProc->usrPtb[0].proc = (PROT_WRITE | PROT_READ);
    idleProc->usrPtb[0].pfn = //TODO Allocate a free frame and give it a number

    //Allocate One page to it
    idleProc->sp = (VMEM_1_LIMIT - pagesize - INITIAL_STACK_FRAME_SIZE);
    idleProc->pc = &DoIdle;
    
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


int SetKernelBrk(void *addr){
    //returns to the kernel lib
    if (m_enableVM){
        //make sure addresses from [addr] to [VMEM_BASE] are valid

        //check no virtual memory in Region 0 out of range are valid

        //Traverse through frames and mapping valid frame with virtual memory
        traverselist(g_freeFrame);

        //Let addr be the new kernel break
    }
    
    //IF ERROR
    return -1;
    //ELSE
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
