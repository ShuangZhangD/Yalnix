#include "kernel.h"
#include "yalnix.h"
#include "listcontrol.h"
#include "pcb.h"
#include "loadprogram.h"

//Global Variables
int m_enableVM = 0; //A flag to check whether Virtual Memory is enabled(1:enabled, 0:not enabled)
int g_pid = 1;

lstnode* currProcnode;
pcb_t* currProc =(pcb_t*) currProcnode->content;
dblist* freeframe_list;
extern dblist* waitingqueue;
extern dblist* readyqueue;
extern dblist* terminatedqueue;



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

int kernelgetpid(){
    return currProc->pid;
}

int kernelbrk(UserContext *uctxt){
    void* addr = (void*)uctxt->regs[0];
    int i,rc;
    unsigned int newBrk = (unsigned int) addr;
    int oldBrkPage = currProc->brk >> PAGESHIFT;
    int sppage = (int)currProc->sp >> PAGESHIFT;
    int newBrkPage = newBrk >> PAGESHIFT;
        
        if(newBrk >= sppage - 1)
        {
            return ERROR;
        }

        if (newBrk > m_kernel_brk){

            for (i = oldBrkPage; i <= newBrk; i++){
                if (!isemptylist(freeframe_list)){
                    g_pageTableR0[i].valid = 1;
                    g_pageTableR0[i].prot = (PROT_READ | PROT_WRITE);
                    lstnode *first = remove_head(freeframe_list);
                    g_pageTableR0[i].pfn = first->id; 
                } else {
                    return ERROR;//TODO clean other 
                }
            }
            //Flush Tlb!
            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

        }else if (newBrk < m_kernel_brk){

            g_pageTableR0[oldBrkPage].valid = 0;

            //TODO REMAP
            for(i = newBrkPage;i <= oldBrkPage;i++){
                lstnode *frame = nodeinit(i);
                insert_tail(frame,freeframe_list);
            }
            //Add this frame back to free frame tracker

            //FLUSH!!!
            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
            if (rc) return ERROR;
        }
        //Let addr be the new kernel break
       currProc->brk = newBrk;


    }



int kerneldelay(UserContext *uctxt){
    int clock_ticks = uctxt->regs[0];
    if (clock_ticks == 0)
    {
        return 0;
    }
    else if(clock_ticks <= 0)
    {
        return ERROR;
    }
    else
    {
        
        enwaitingqueue(currProcnode,waitingqueue);
        currProc->clock = clock_ticks;
        if (!isemptylist(waitingqueue))
        {
            currProcnode = dereadyqueue(readyqueue);
        }
        else{
            //currProcnode = idleProc;
        }
    }

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
    TracePrintf(1, "m_kernel_brk = %x \n", m_kernel_brk);
    m_kernel_data_start = (unsigned int) _KernelDataStart;

    return;
}

int SetKernelBrk(void *addr){
    //returns to the kernel lib
    unsigned int newBrk = (unsigned int) addr;

    TracePrintf(1, "Being called ! addr = %x, m_enableVM = %d\n", addr, m_enableVM);

    int i,rc = 0;
    int newBrkPage = newBrk >> PAGESHIFT;
    int oldBrkPage = m_kernel_brk >> PAGESHIFT;
    if (m_enableVM){

        rc = checkPageStatus(newBrk);
        if (rc) return -1;
        
        if (newBrk > m_kernel_brk){

            for (i = oldBrkPage; i <= newBrk; i++){
                if (!isemptylist(freeframe_list)){
                    g_pageTableR0[i].valid = 1;
                    g_pageTableR0[i].prot = (PROT_READ | PROT_WRITE);
                    lstnode *first = remove_head(freeframe_list);
                    g_pageTableR0[i].pfn = first->id; 
                } else {
                    return -1;//TODO clean other 
                }
            }
            //Flush Tlb!
            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

        }else if (newBrk < m_kernel_brk){

            g_pageTableR0[oldBrkPage].valid = 0;

            //TODO REMAP
            for(i = newBrkPage;i <= oldBrkPage;i++){
                lstnode *frame = nodeinit(i);
                insert_tail(frame,freeframe_list);
            }
            //Add this frame back to free frame tracker

            //FLUSH!!!
            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
            if (rc) return -1;
        }
        //Let addr be the new kernel break
    
    }

    m_kernel_brk = newBrk;
    TracePrintf(1, "New Brk! m_kernel_brk = %x\n", m_kernel_brk);
    return 0;
}


void KernelStart(char *cmd_args[],unsigned int pmem_size, UserContext *uctxt){
    int i, rc;
    //Initialize interrupt vector table and REG_VECTOR_BASE
    TracePrintf(1, "Init interrupt table.\n");
    InitInterruptTable();

    //Build a structure to track free frame
    TracePrintf(1, "Init free frame list.\n");
    initFreeFrameTracking(pmem_size);

    //Build initial page table for Region 1 (before kernel page protection)
    TracePrintf(1, "Init user page table \n");
    pte_t* usrPtb = InitUserPageTable();

    //Build initial page table for Region 0
    TracePrintf(1, "Init kernel page table \n");
    InitKernelPageTable();
    WriteRegister(REG_PTBR0, (unsigned int) g_pageTableR0);
    WriteRegister(REG_PTLR0, (unsigned int) MAX_PT_LEN);

    WriteRegister(REG_PTBR1, (unsigned int) usrPtb);
    WriteRegister(REG_PTLR1, (unsigned int) MAX_PT_LEN);

    TracePrintf(1, "Enable VM\n");
    WriteRegister(REG_VM_ENABLE,1);
    m_enableVM = 1;

    //init Queue
    waitingqueue = listinit();
    readyqueue = listinit();
    terminatedqueue = listinit();

   //====Cook DoIdle()====
    TracePrintf(1, "DoIdle\n");
    // CookDoIdle(uctxt);

    //Initialize New Process    
    TracePrintf(1, "Init pcb.\n");
    pcb_t *idleProc = InitIdleProc(uctxt);

    pcb_t *initProc = InitIdleProc(uctxt);

    //Create first process  and load initial program to it
    rc = LoadProgram("init", cmd_args, initProc);
    TracePrintf(1, "rc=%d\n", rc);
    if (rc == KILL){
        //TODO Kill the process
    }

    rc = KernelContextSwitch(MyKCS, (void *) &idleProc, (void *) &initProc);

    TracePrintf(1, "Exit\n");
    return;
}

//  =========================================================================



pte_t* InitUserPageTable (){
    int i;
    pte_t *usrPtb = (pte_t *) malloc(sizeof(pte_t) * MAX_PT_LEN);
    //Mark User Page table as Invalid;
    for (i = 0; i < MAX_PT_LEN; i++){
        usrPtb[i].valid = 0;
        // usrPtb[i].prot = PROT_NONE;
        // usrPtb[i].pfn = UNALLOCATED;
    }

    // idleProc->usrPtb[MAX_PT_LEN - 1].pfn = nodeinit(i);
    // remove_node(frame, freeframe_list);

    return usrPtb;
}


void InitKernelPageTable(pcb_t *proc) {
    
    unsigned int kDataEdPage = m_kernel_brk >> PAGESHIFT; 
    unsigned int kDataStPage = m_kernel_data_start >> PAGESHIFT;
    unsigned int kStackStPage = KERNEL_STACK_BASE >> PAGESHIFT;
    unsigned int kStackEdPage = (KERNEL_STACK_LIMIT - 1) >> PAGESHIFT;
    int i, stackInx;
    

    //Protect Kernel Text, Data and Heap
    for (i=0; i <= kDataEdPage; i++){
        if (i < kDataStPage){
            //Protect Kernel Text
            g_pageTableR0[i].valid = 1;
            g_pageTableR0[i].prot = (PROT_READ | PROT_EXEC);
            g_pageTableR0[i].pfn = i;
            remove_node(i, freeframe_list);
        } else {
            //Protect Kernel Data & Heap
            g_pageTableR0[i].valid = 1;
            g_pageTableR0[i].prot = (PROT_READ | PROT_WRITE);
            g_pageTableR0[i].pfn = i;
            remove_node(i, freeframe_list);
        }
    }

    //Protect Kernel Stack
    for (i=kStackStPage, stackInx = 0; i <= kStackEdPage; i++, stackInx++){
        g_pageTableR0[i].valid = 1;
        g_pageTableR0[i].prot = (PROT_READ | PROT_WRITE);
        g_pageTableR0[i].pfn = i;
        remove_node(i, freeframe_list);
    }
    
    return;

}

//Do Idle Process
void DoIdle (void){
    while(1){
        TracePrintf(1, "DoIdle\n");
        Pause();
    }
    return;
}

void CookDoIdle(UserContext *uctxt){

    //Get the function pointer of DoIdle
    void (*idlePtr)(void) = &DoIdle;
    uctxt->pc = idlePtr;
    uctxt->sp = (void*)(KERNEL_STACK_LIMIT  - INITIAL_STACK_FRAME_SIZE - POST_ARGV_NULL_SPACE); 
    return;
}


void initFreeFrameTracking(int pmem_size){
    int i;
    int numOfFrames = (pmem_size / PAGESIZE);
    freeframe_list = listinit();

    for(i = 0;i<numOfFrames;i++)
    {
        lstnode *frame = nodeinit(i);
        insert_tail(frame,freeframe_list);
    }
    return;
}


pcb_t *InitIdleProc(UserContext *uctxt){
    int i, stackInx;
    int numOfStack = KERNEL_STACK_MAXSIZE / PAGESIZE;
    unsigned int kStackStPage = KERNEL_STACK_BASE >> PAGESHIFT;
    unsigned int kStackEdPage = KERNEL_STACK_LIMIT >> PAGESHIFT;

    //Initialize Queue TODO Jason Please finish this! At least finish running queue!
    
    //Initialize Process
    pcb_t *proc = (pcb_t *) malloc (sizeof(pcb_t));
    proc->procState = RUNNING;
    proc->pid = g_pid++;
    proc->uctxt = *uctxt;

    proc->krnlStackPtb = (pte_t *) calloc(numOfStack ,sizeof(pte_t));
    proc->krnlStackPtbSize = numOfStack;
    lstnode* procnode = listinit(proc->pid);
    //Let a userprocess have its own kernel stack
    for (i=kStackStPage, stackInx = 0; i<kStackEdPage; i++, stackInx++){
        proc->krnlStackPtb[stackInx] = g_pageTableR0[i];
    }

    return proc;
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


/*
    Context Switch
*/
// when someone calls KernelContextSwitch, it might come to here.
KernelContext *MyKCS(KernelContext *kc_in,void *curr_pcb_p,void *next_pcb_p){
    int numOfStack = KERNEL_STACK_MAXSIZE / PAGESIZE;
    int kStackStPage = (KERNEL_STACK_BASE >> PAGESHIFT);
    int kStackEdPage = (KERNEL_STACK_LIMIT >> PAGESHIFT);
    int i, stackInx;

    pcb_t *cur_p = (pcb_t *) curr_pcb_p;
    pcb_t *next_p = (pcb_t *) next_pcb_p;

    //Copy the kernel context to current process's pcb
    cur_p->kctxt = *kc_in;

    //Remember to change page table entries for kernel stack
    for (i = kStackStPage, stackInx = 0; i <= kStackEdPage; i++, stackInx++){
        memcpy(g_pageTableR0[i], next_p->krnlStackPtb[stackInx], sizeof(pte_t));
    }

    //Return a pointer to a kernel context it had earlier saved
    return &next_p->kctxt;

}

