#include "kernel.h"
#include "yalnix.h"
#include "listcontrol.h"
#include "processmanage.h"
#include "loadprogram.h"
#include "io.h"

//Global Variables
int m_enableVM = 0; //A flag to check whether Virtual Memory is enabled(1:enabled, 0:not enabled)
int g_pid = 1;
int g_isInitial = 1;
int const g_pageNumOfStack = KERNEL_STACK_MAXSIZE / PAGESIZE;
int const g_kStackStPage = KERNEL_STACK_BASE >> PAGESHIFT;
int const g_kStackEdPage = (KERNEL_STACK_LIMIT - 1) >> PAGESHIFT;

lstnode* currProc;
dblist* freeframe_list;

extern dblist* waitingqueue;
extern dblist* readyqueue;
extern Tty* tty[NUM_TERMINALS];

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
    TracePrintf(1, "_KernelDataStart: %p\n", _KernelDataStart);
    m_kernel_brk = (unsigned int) _KernelDataEnd;
    m_kernel_data_end = (unsigned int) _KernelDataEnd;
    m_kernel_data_start = (unsigned int) _KernelDataStart;


    TracePrintf(1, "KernelDataStart = %x \n", m_kernel_data_start);
    TracePrintf(1, "KernelDataEnd = %x \n", m_kernel_brk);  
    return;
}

int SetKernelBrk(void *addr){
    TracePrintf(1, "SetKernelBrk is called ! addr = %x, m_enableVM = %d\n", addr, m_enableVM);

    int i,rc = 0;
    unsigned int newBrk = (unsigned int) addr;
    int newBrkPage = newBrk >> PAGESHIFT;
    int oldBrkPage = m_kernel_brk >> PAGESHIFT;
    if (m_enableVM){

        rc = checkPageStatus(newBrk);
        if (rc) return -1;

        if (newBrk > m_kernel_brk){
            writepagetable(g_pageTableR0, oldBrkPage, newBrkPage, VALID, (PROT_READ | PROT_WRITE));
            //Flush Tlb!
            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

        }else if (newBrk < m_kernel_brk){
            //Remap  Add this frame back to free frame tracker
            ummap(g_pageTableR0, newBrkPage, oldBrkPage, INVALID, PROT_NONE);
            //FLUSH!!!
            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
        }
    }

    //Let addr be the new kernel break
    m_kernel_brk = newBrk;
    TracePrintf(1, "New Brk! m_kernel_brk = %x\n", m_kernel_brk);
    return 0;
}


void KernelStart(char *cmd_args[],unsigned int pmem_size, UserContext *uctxt){
    int i, rc, stackInx = 0;
    //Initialize interrupt vector table and REG_VECTOR_BASE
    TracePrintf(1, "Init interrupt table.\n");
    InitInterruptTable();

    //Build a structure to track free frame
    TracePrintf(1, "Init free frame list.\n");
    InitFreeFrameTracking(pmem_size);

    //Build initial page table for Region 1 (before kernel page protection)
    TracePrintf(1, "Init user page table \n");
    pte_t *idlePageTable = InitUserPageTable();

    // //Build initial page table for Region 0
    TracePrintf(1, "Init kernel page table \n");
    InitKernelPageTable();
    WriteRegister(REG_PTBR0, (unsigned int) g_pageTableR0);
    WriteRegister(REG_PTLR0, (unsigned int) MAX_PT_LEN);

    WriteRegister(REG_PTBR1, (unsigned int) idlePageTable);
    WriteRegister(REG_PTLR1, (unsigned int) MAX_PT_LEN);


    TracePrintf(1, "Enable VM\n");
    WriteRegister(REG_VM_ENABLE,1);
    m_enableVM = 1;

    //init Queue
    waitingqueue = listinit();
    readyqueue = listinit();

    for (i = 0; i < NUM_TERMINALS; i++)
    {
        tty[i] = (Tty*) malloc(sizeof(Tty));
        tty[i]->readerwaiting = listinit();
        tty[i]->writerwaiting = listinit();
        //tty[i]->receivebuf = (char*) malloc(sizeof(char) * TERMINAL_MAX_LINE);
        //tty[i]->transmitbuf = (char*) malloc(sizeof(char) * TERMINAL_MAX_LINE);
    }
    blockqueue = listinit();

    //Initialize Idle Process    
    TracePrintf(1, "Init pcb.\n");
    pcb_t *idlePcb = InitIdleProc(uctxt);
    idlePcb->usrPtb = idlePageTable;
    idlePcb->usrPtb[MAX_PT_LEN-1].valid = 1;
    idlePcb->usrPtb[MAX_PT_LEN-1].prot = (PROT_WRITE | PROT_READ);
    idlePcb->usrPtb[MAX_PT_LEN-1].pfn = remove_head(freeframe_list)->id;

    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

    //====Cook DoIdle()====
    TracePrintf(1, "Cook DoIdle\n");
    CookDoIdle(uctxt);

    idlePcb->uctxt = *uctxt;
    lstnode *idleProc = TurnPCBToNode(idlePcb);
    
    //Initialize Init Process
    lstnode *initProc = InitProc();

    //Create first process  and load initial program to it
    if (NULL == cmd_args[0]){
        rc = LoadProgram("init", cmd_args, initProc);
    } else {
        rc = LoadProgram(cmd_args[0], cmd_args, initProc); 
    }

    if (rc == KILL){
        terminateProcess(initProc);
        return;
    }

    // enreadyqueue(initProc, readyqueue);
    enreadyqueue(idleProc, readyqueue);

    currProc = initProc;

    TracePrintf(1,"idle:%p, init:%p\n", idlePcb->usrPtb, TurnNodeToPCB(initProc)->usrPtb);
    rc = KernelContextSwitch(MyCloneKCS, (void *)idleProc, (void *)initProc);

    if (rc) {
        TracePrintf(1, "Context Switch in KernelStart goes wrong.\n");
    }

    *uctxt = TurnNodeToPCB(currProc)->uctxt;
    TracePrintf(1,"Exit KernelStart.\n");
    return;
}   

//  =========================================================================

pte_t* InitUserPageTable (){
    int i;

    pte_t *usrPtb = (pte_t *) malloc(sizeof(pte_t) * MAX_PT_LEN);

    //Mark User Page table as Invalid;
    for (i = 0; i < MAX_PT_LEN; i++){
        usrPtb[i].valid = 0;
        usrPtb[i].prot = PROT_NONE;
        usrPtb[i].pfn = UNALLOCATED;
    }


    return usrPtb;
}


void InitKernelPageTable(pcb_t *proc) {

    unsigned int kDataStPage = m_kernel_data_start >> PAGESHIFT;
    unsigned int kDataEdPage = m_kernel_brk >> PAGESHIFT; 
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
    for (i=g_kStackStPage, stackInx = 0; i <= g_kStackEdPage; i++, stackInx++){
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
    uctxt->sp = (void*) (VMEM_1_LIMIT - INITIAL_STACK_FRAME_SIZE - POST_ARGV_NULL_SPACE); 
    uctxt->addr = uctxt->sp; 
    return;
}


void InitFreeFrameTracking(int pmem_size){
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

lstnode *InitProc(){
    int i, stackInx;

    //Initialize Process
    pcb_t *proc = (pcb_t *) malloc (sizeof(pcb_t));
    proc->procState = READY;
    proc->pid = g_pid++;

    proc->usrPtb = InitUserPageTable();
    // proc->uctxt = *uctxt;

    proc->krnlStackPtb = (pte_t *) calloc(g_pageNumOfStack ,sizeof(pte_t));
    proc->krnlStackPtbSize = g_pageNumOfStack;

    //Let a userprocess have its own kernel stack
    for (i=g_kStackStPage, stackInx = 0; i<=g_kStackEdPage; i++, stackInx++){
        proc->krnlStackPtb[stackInx].prot = g_pageTableR0[i].prot;
        proc->krnlStackPtb[stackInx].valid = g_pageTableR0[i].valid;

        lstnode *first = remove_head(freeframe_list);
        proc->krnlStackPtb[stackInx].pfn = first->id;
    }

    proc->children = listinit();
    proc->terminatedchild = listinit();

    return TurnPCBToNode(proc);
}

pcb_t *InitIdleProc(UserContext *uctxt){
    int i, stackInx;

    //Initialize Process
    pcb_t *proc = (pcb_t *) malloc (sizeof(pcb_t));
    proc->procState = READY;
    proc->pid = g_pid++;
    proc->uctxt = *uctxt;

    proc->krnlStackPtb = (pte_t *) calloc(g_pageNumOfStack ,sizeof(pte_t));
    proc->krnlStackPtbSize = g_pageNumOfStack;

    //Let a userprocess have its own kernel stack
    for (i=g_kStackStPage, stackInx = 0; i<=g_kStackEdPage; i++, stackInx++){
        proc->krnlStackPtb[stackInx].pfn = g_pageTableR0[i].pfn;
        proc->krnlStackPtb[stackInx].prot = g_pageTableR0[i].prot;
        proc->krnlStackPtb[stackInx].valid = g_pageTableR0[i].valid;
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
    for (i = pageAddr; i <= SAFETY_MARGIN_PAGE; i++){
        if (1 == g_pageTableR0[i].valid) return -1;
    }

    if (addr < m_kernel_data_end) return -1;

    return 0;
}

void CopyKernelStack (pte_t* pageTable){
    int i, kernelSize = g_pageNumOfStack;

    //Use a safety margin page as a buffer of copying memory
    g_pageTableR0[SAFETY_MARGIN_PAGE].valid = VALID;
    g_pageTableR0[SAFETY_MARGIN_PAGE].prot = (PROT_READ | PROT_WRITE);

    for (i = 0; i < kernelSize; i++){
        g_pageTableR0[SAFETY_MARGIN_PAGE].pfn = pageTable[i].pfn;

        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

        unsigned int desAddr = SAFETY_MARGIN_PAGE << PAGESHIFT;
        unsigned int srcAddr = (g_kStackStPage + i) << PAGESHIFT;

        memcpy((void *)desAddr, (void *)srcAddr, PAGESIZE);
    }
  
    //Restore the buffer.
    g_pageTableR0[SAFETY_MARGIN_PAGE].valid = INVALID;
    g_pageTableR0[SAFETY_MARGIN_PAGE].prot = PROT_NONE;
    g_pageTableR0[SAFETY_MARGIN_PAGE].pfn = UNALLOCATED;

    return;
}

KernelContext *MyCloneKCS(KernelContext *kc_in,void *curNode,void *nxtNode){
    TracePrintf(1, "Enter MyCloneKCS\n");

    int i, stackInx = 0;

    lstnode* cur_node = (lstnode*) curNode;
    lstnode* nxt_node = (lstnode*) nxtNode;
    pcb_t *cur_pcb = TurnNodeToPCB(cur_node);
    pcb_t *nxt_pcb = TurnNodeToPCB(nxt_node);

    CopyKernelStack(nxt_pcb->krnlStackPtb);
    //Remember to change page table entries for kernel stack
    for (i = g_kStackStPage; i <= g_kStackEdPage; i++){
        g_pageTableR0[i].pfn = nxt_pcb->krnlStackPtb[stackInx].pfn;
        stackInx++;
    }

    //Flush All TLB because 1. Kernel Stack Mapping has changed
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

    //Copy the kernel context to current process's pcb
    cur_pcb->kctxt = *kc_in;
    nxt_pcb->kctxt = *kc_in;

    TracePrintf(1, "Exit MyCloneKCS\n");
    return kc_in;
}


// when someone calls KernelContextSwitch, it might come to here.
KernelContext *MyTrueKCS(KernelContext *kc_in,void *curNode,void *nxtNode){
    TracePrintf(1,"Enter MyTrueKCS\n");

    int i ,stackInx = 0;

    lstnode* curr_pcb_node = (lstnode*) curNode;
    lstnode* next_pcb_node = (lstnode*) nxtNode;
    
    pcb_t *cur_p = TurnNodeToPCB(curr_pcb_node);
    pcb_t *next_p = TurnNodeToPCB(next_pcb_node);

    //Copy the kernel context to current process's pcb
    cur_p->kctxt = *kc_in;

    //Remember to change page table entries for kernel stack
    for (i = g_kStackStPage; i <= g_kStackEdPage; i++){
        g_pageTableR0[i].pfn = next_p->krnlStackPtb[stackInx].pfn;
        stackInx++;
    }

    WriteRegister(REG_PTBR1, (unsigned int) next_p->usrPtb);
    WriteRegister(REG_PTLR1, (unsigned int) MAX_PT_LEN);

    //Flush All TLB because 1. Kernel Stack Mapping has changed 2. User Page Table has been written into register
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

    //Turn Next Process to be the current one
    if (READY == cur_p->procState){
        enreadyqueue(curr_pcb_node ,readyqueue);
    }

    lstnode* node = dereadyqueue(readyqueue);
    if (node != next_pcb_node) TracePrintf(1, "KernelContextSwitch Error!");
    currProc = node; 

    TracePrintf(1,"Exit MyTrueKCS\n");
    //Return a pointer to a kernel context it had earlier saved
    return &(next_p->kctxt);
}

KernelContext *MyIOKCS(KernelContext *kc_in,void *curNode,void *nxtNode){
    TracePrintf(1,"Enter MyIOKCS\n");

    int i ,stackInx = 0;

    lstnode* curr_pcb_node = (lstnode*) curNode;
    lstnode* next_pcb_node = (lstnode*) nxtNode;
    
    pcb_t *cur_p = TurnNodeToPCB(curr_pcb_node);
    pcb_t *next_p = TurnNodeToPCB(next_pcb_node);

    //Copy the kernel context to current process's pcb
    cur_p->kctxt = *kc_in;

    //Remember to change page table entries for kernel stack
    for (i = g_kStackStPage; i <= g_kStackEdPage; i++){
        g_pageTableR0[i].pfn = next_p->krnlStackPtb[stackInx].pfn;
        stackInx++;
    }

    WriteRegister(REG_PTBR1, (unsigned int) next_p->usrPtb);
    WriteRegister(REG_PTLR1, (unsigned int) MAX_PT_LEN);

    //Flush All TLB because 1. Kernel Stack Mapping has changed 2. User Page Table has been written into register
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

    currProc = next_pcb_node; 

    TracePrintf(1,"Exit MyIOKCS\n");
    //Return a pointer to a kernel context it had earlier saved
    return &(next_p->kctxt);
}

KernelContext *MyTerminateKCS(KernelContext *kc_in,void *termNode,void *nxtNode){
    
    int i, stackInx = 0;

    lstnode* term_pcb_node = (lstnode*) termNode;
    lstnode* next_pcb_node = (lstnode*) nxtNode;
    
    pcb_t *term_p = TurnNodeToPCB(term_pcb_node);
    pcb_t *next_p = TurnNodeToPCB(next_pcb_node);

/////////////////
    // CopyKernelStack(next_p->krnlStackPtb);
    //Remember to change page table entries for kernel stack
    for (i = g_kStackStPage; i <= g_kStackEdPage; i++){
        g_pageTableR0[i].pfn = next_p->krnlStackPtb[stackInx].pfn;
        stackInx++;
    }

    WriteRegister(REG_PTBR1, (unsigned int) next_p->usrPtb);
    WriteRegister(REG_PTLR1, (unsigned int) MAX_PT_LEN);
    //Flush All TLB because 1. Kernel Stack Mapping has changed
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

/////////////////


    emptyregion1pagetable(term_p);

    // If it has children, they should run normally but without a parent
    lstnode* traverse = term_p->children->head->next;
    while(traverse != NULL && traverse->id != -1) {
        pcb_t* proc = TurnNodeToPCB(traverse);
        proc->parent = NULL;             
        traverse = traverse->next;   
    }

    //When the orphans later exit, you need not save or report their exit status since there is no longer anybody to care.
    if (NULL != term_p->parent){
        pcb_t* currParent = TurnNodeToPCB(term_p->parent);

        if (NULL != search_node(currParent->pid,blockqueue)){
            lstnode* node = deblockqueue(term_p->parent,blockqueue);
            TracePrintf(1,"term_p->parent->id:%d\n", term_p->parent->id);
            TracePrintf(1,"node->id:%d\n", node->id);
            enreadyqueue(node,readyqueue);       
        }

        lstnode* node = remove_node(term_p->pid, currParent->children);
        
        free(term_p->usrPtb);
        free(term_p->krnlStackPtb); 
        free(term_p->children);
        free(term_p->terminatedchild);

        insert_tail(node,currParent->terminatedchild);
    } 

    free(term_pcb_node);

    lstnode* node = dereadyqueue(readyqueue);
    if (node != next_pcb_node) TracePrintf(1, "KernelContextSwitch Error!");
    currProc = node;

    TracePrintf(1,"Exit MyTerminateKCS\n");

    //Return a pointer to a kernel context it had earlier saved
    return &(next_p->kctxt);

}


KernelContext *MyForkKCS(KernelContext *kc_in,void *curNode,void *nxtNode){
    TracePrintf(1, "Enter MyTestKCS\n");

    int i, stackInx = 0;

    lstnode* cur_node = (lstnode*) curNode;
    lstnode* nxt_node = (lstnode*) nxtNode;
    pcb_t *cur_pcb = TurnNodeToPCB(cur_node);
    pcb_t *nxt_pcb = TurnNodeToPCB(nxt_node);

    CopyKernelStack(nxt_pcb->krnlStackPtb);

    //Flush All TLB because 1. Kernel Stack Mapping has changed
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

    //Copy the kernel context to current process's pcb
    cur_pcb->kctxt = *kc_in;
    nxt_pcb->kctxt = *kc_in;

    TracePrintf(1, "Exit MyTestKCS\n");
    return kc_in;
}
