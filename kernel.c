#include"kernel.h"
#include"yalnix.h"
#include"hardware.h"
#include"loadprogram.h"

int g_enableVM = 0; //A flag to check whether Virtual Memory is enabled(1:enabled, 0:not enabled)
dblist* g_freeFrame;

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
    m_kernel_brk = _KernelDataEnd;
    m_kernel_data_start = _KernelDataStart;

    return;
}

void KernelStart(char *cnd_args[],unsigned int pmem_size, UserContext *uctxt){

    //Initialize interrupt vector table and REG_VECTOR_BASE
    InitInterruptTable();
    
    //Build a structure to track free frame
    g_freeFrame = listinit();

    int numOfFrames = (pmem_size / PAGESIZE);
    //keep track of free frame;
    //TODO
    
    int kDataEdPage = ; 
    int kDataStPage = ;
        
    // 

    //Build initial page for Region 0 and Region 1;
    WriteRegister(REG_PTBR0, (unsigned int) base0);
    WriteRegister(REG_PTLR0, (unsigned int) limit0);

    WriteRegister(REG_PTBR0, (unsigned int) base1);
    WriteRegister(REG_PTLR0, (unsigned int) limit1);

    // enable virtual memory
    WriteRegister(REG_VM_ENABLE,1);
    g_enableVM = 1;
    //Create idle process

    //Create first process  and load initial program to it
    loadprogram(char *name, char *args[], proc);

    return;
}

int SetKernelBrk(void *addr){
    //returns to the kernel lib
    if (g_enableVM){
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
    //Copy the kernel context to current process's pcb
    (PCB*) curr_pcb_p->kernelcontext = *kc_in;

    //Remember to change page table entries for kernel stack

    //Return a pointer to a kernel context it had earlier saved
    return (PCB* )next_pcb_p->kernelcontext;

}
