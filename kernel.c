#include "kernel.h"
#include "yalnix.h"
#include "listcontrol.h"
#include "pcb.h"
#include "loadprogram.h"

//Global Variables
int m_enableVM = 0; //A flag to check whether Virtual Memory is enabled(1:enabled, 0:not enabled)
int g_pid = 1;
int const g_pageNumOfStack = KERNEL_STACK_MAXSIZE / PAGESIZE;
int const g_kStackStPage = KERNEL_STACK_BASE >> PAGESHIFT;
int const g_kStackEdPage = (KERNEL_STACK_LIMIT - 1) >> PAGESHIFT;

lstnode* currProc;

dblist* freeframe_list;

extern dblist* waitingqueue;
extern dblist* readyqueue;
extern dblist* terminatedqueue;

int kernelfork(UserContext *uctxt){
    lstnode* child; //= initProc(currProc); //TODO Replace new initproc
    pcb_t* childproc = (pcb_t*) child->content;
    pcb_t* parentproc = (pcb_t*)currProc->content;

    childproc->parent = currProc;

    int i, stackInx;

    //Initialize Process
    pcb_t *proc = (pcb_t *) malloc (sizeof(pcb_t));
    proc->pid = g_pid++;

    proc->krnlStackPtb = (pte_t *) calloc(g_pageNumOfStack ,sizeof(pte_t));
    proc->krnlStackPtbSize = g_pageNumOfStack;

    proc->usrPtb = InitUserPageTable();

    // pte_t *usrPtb = (pte_t *) malloc(sizeof(pte_t) * MAX_PT_LEN);

    //Mark User Page table as Invalid;
    for (i = 0; i < MAX_PT_LEN; i++){
        childproc->usrPtb[i].valid = 0;
        childproc->usrPtb[i].prot = PROT_NONE;
        childproc->usrPtb[i].pfn = 0; //TODO make sure it is right
    }


    memcpy();

    proc->krnlStackPtb = (pte_t *) calloc(g_pageNumOfStack ,sizeof(pte_t));
    proc->krnlStackPtbSize = g_pageNumOfStack;

    //Let a userprocess have its own kernel stack
    for (i=g_kStackStPage, stackInx = 0; i<=g_kStackEdPage; i++, stackInx++){
        proc->krnlStackPtb[stackInx] = g_pageTableR0[i];
    }


    // return TurnPCBToNode(proc);
    return 0; // TODO

    insert_tail(child,proc->children);

    enreadyqueue(currProc,waitingqueue);

    if(currProc == child)
    {
        return 0;
    }
    else{
        return childproc->pid;
    }

    return ERROR;
}

int kernelexec(UserContext *uctxt){
    pcb_t *proc = (pcb_t*)currProc->content;
    char* name = (char*) uctxt->regs[0];
    char** args = (char**) uctxt->regs[1];
    int rc = LoadProgram(name,args,currProc);
    //todo
    if (rc == 0)
    {
        return 0;
    }
    if (rc == ERROR)
    {
        return ERROR;
    }
}

int kernelexit(UserContext *uctxt){
    pcb_t *proc = (pcb_t*)currProc->content;
    int status = uctxt->regs[0];

    if(proc->pid == 2)
    {
        Halt();
    }

    lstnode* traverse = proc->children->head;
    while(traverse != NULL)
    {               
        pcb_t* proc = (pcb_t*) traverse->content;
        proc->parent = NULL;
        traverse = traverse->next;   
    }

    if(proc->parent != NULL)
    {
        pcb_t* pcb = (pcb_t *) proc->parent->content;
        pcb->terminatedchild = listinit();
        remove_node(proc->pid, proc->children);
        insert_tail(currProc,pcb->terminatedchild);
        dewaitingqueue(proc->parent,waitingqueue);
        enreadyqueue(proc->parent,readyqueue); 
    }


    proc->exitstatus = status;
    switchproc();

}

int kernelwait(UserContext *uctxt){
    pcb_t *proc = (pcb_t*)currProc->content;

    if (isemptylist(proc->children) && isemptylist(proc->terminatedchild))
    {
        return ERROR;
    }
    if (!isemptylist(proc->terminatedchild))
    {
        lstnode* remove = remove_head(proc->terminatedchild);
        pcb_t* removeproc = (pcb_t*) remove->content;
        uctxt->regs[0] = removeproc->exitstatus;
        return removeproc->pid;
    }
    else{
        enwaitingqueue(currProc,waitingqueue);
        switchproc();
    }

}

int kernelgetpid(){
    return TurnNodeToPCB(currProc)->pid;
}

int kernelbrk(UserContext *uctxt){
    int i,rc;
    pcb_t *proc = TurnNodeToPCB(currProc);

    unsigned int newBrk = (unsigned int) uctxt->regs[0];
    int oldBrkPage = proc->brk;
    int stacklimitpage = proc->stack_limit;
    int newBrkPage = newBrk >> PAGESHIFT;

    if(newBrkPage >= stacklimitpage - 1){
        return ERROR;
    }

    if (newBrkPage > oldBrkPage){
        writepagetable(proc->usrPtb, oldBrkPage, newBrkPage, VALID, (PROT_READ | PROT_WRITE));
        //Flush Tlb!
        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

    }else if (newBrk < m_kernel_brk){
        //Remap  Add this frame back to free frame tracker
        ummap(proc->usrPtb, newBrkPage, oldBrkPage, INVALID, PROT_NONE);
        //FLUSH!!!
        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
    }else {
        //Do nothing
    }

    //Let addr be the new kernel break
    proc->brk = newBrkPage;

    return SUCCESS;
}



int kerneldelay(UserContext *uctxt){
    TracePrintf(1, "Enter KernelDelay\n");

    pcb_t *proc = TurnNodeToPCB(currProc);
    int clock_ticks = uctxt->regs[0];
    int rc;
    if (clock_ticks == 0){
        return 0;
    }
    else if(clock_ticks <= 0){
        return ERROR;
    }
    else{
        enwaitingqueue(currProc,waitingqueue);
        proc->clock = clock_ticks;
        rc = switchproc();
        if (rc) {
            return ERROR;
        }
    }
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
    TracePrintf(1, "_KernelDataStart: %p\n", _KernelDataStart);
    m_kernel_brk = (unsigned int) _KernelDataEnd;
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
            writepagetable(g_pageTableR0, oldBrkPage, newBrk, VALID, (PROT_READ | PROT_WRITE));
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
    int i, rc;
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
    terminatedqueue = listinit();

    //Initialize Idle Process    
    TracePrintf(1, "Init pcb.\n");
    pcb_t *idlePcb = InitIdleProc(uctxt);
    idlePcb->usrPtb = idlePageTable;
    idlePcb->usrPtb[MAX_PT_LEN-1].valid = 1;
    idlePcb->usrPtb[MAX_PT_LEN-1].prot = (PROT_WRITE | PROT_READ);
    idlePcb->usrPtb[MAX_PT_LEN-1].pfn = remove_head(freeframe_list)->id;

    //====Cook DoIdle()====
    TracePrintf(1, "Cook DoIdle\n");
    CookDoIdle(uctxt);

    idlePcb->uctxt = *uctxt;
    lstnode *idleProc = TurnPCBToNode(idlePcb);
    
    enreadyqueue(idleProc, readyqueue);

    //Initialize Init Process
    lstnode *initProc = InitProc();

    //Create first process  and load initial program to it
    rc = LoadProgram("init", cmd_args, initProc);
    TracePrintf(1, "rc=%d\n", rc);
    if (rc == KILL){
        terminateProcess(initProc);
    }

    currProc = initProc;

    rc = KernelContextSwitch(MyCloneKCS, (void *)initProc, (void *)idleProc);
    if (rc) {
        TracePrintf(1, "Context Switch in KernelStart goes wrong.\n");
    }

    *uctxt = TurnNodeToPCB(currProc)->uctxt;
    TracePrintf(1, "Exit KernelStart\n");
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
        usrPtb[i].pfn = UNALLOCATED; //TODO make sure it is right
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
    proc->procState = RUNNING;
    proc->pid = g_pid;

    proc->usrPtb = InitUserPageTable();

    proc->krnlStackPtb = (pte_t *) calloc(g_pageNumOfStack ,sizeof(pte_t));
    proc->krnlStackPtbSize = g_pageNumOfStack;

    //Let a userprocess have its own kernel stack
    for (i=g_kStackStPage, stackInx = 0; i<=g_kStackEdPage; i++, stackInx++){
        proc->krnlStackPtb[stackInx] = g_pageTableR0[i];
    }

    return TurnPCBToNode(proc);
}

pcb_t *InitIdleProc(UserContext *uctxt){
    int i, stackInx;

    //Initialize Process
    pcb_t *proc = (pcb_t *) malloc (sizeof(pcb_t));
    proc->procState = RUNNING;
    proc->pid = g_pid++;
    proc->uctxt = *uctxt;

    proc->krnlStackPtb = (pte_t *) calloc(g_pageNumOfStack ,sizeof(pte_t));
    proc->krnlStackPtbSize = g_pageNumOfStack;

    lstnode* procnode = nodeinit(proc->pid);
    //Let a userprocess have its own kernel stack
    for (i=g_kStackStPage, stackInx = 0; i<=g_kStackEdPage; i++, stackInx++){
        proc->krnlStackPtb[stackInx] = g_pageTableR0[i];
        
        lstnode *first = remove_head(freeframe_list);
        proc->krnlStackPtb[stackInx].pfn = first->id;
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

    return 0;
}


KernelContext *MyCloneKCS(KernelContext *kc_in,void *curNode,void *nxtNode){
    TracePrintf(1, "Enter MyCloneKCS\n");
    lstnode* cur_node = (lstnode*) curNode;
    lstnode* nxt_node = (lstnode*) nxtNode;
    pcb_t *cur_pcb = TurnNodeToPCB(cur_node);
    pcb_t *nxt_pcb = TurnNodeToPCB(nxt_node);
    
    int i, kernelSize = g_pageNumOfStack;

    //Use a safety margin page as a buffer of copying memory
    g_pageTableR0[SAFETY_MARGIN_PAGE].valid = 1;
    g_pageTableR0[SAFETY_MARGIN_PAGE].prot = (PROT_READ | PROT_WRITE);

    for (i = 0; i < kernelSize; i++){
        g_pageTableR0[SAFETY_MARGIN_PAGE].pfn = nxt_pcb->krnlStackPtb[i].pfn;

        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

        unsigned int desAddr = SAFETY_MARGIN_PAGE << PAGESHIFT;
        unsigned int srcAddr = (g_kStackStPage + i) << PAGESHIFT;

        memcpy((void *)desAddr, (void *)srcAddr, PAGESIZE);
    }

    //Restore the buffer.
    g_pageTableR0[SAFETY_MARGIN_PAGE].valid = 0;
    g_pageTableR0[SAFETY_MARGIN_PAGE].prot = PROT_NONE;
    g_pageTableR0[SAFETY_MARGIN_PAGE].pfn = UNALLOCATED;

    //Copy the kernel context to current process's pcb
    cur_pcb->kctxt = *kc_in;
    nxt_pcb->kctxt = *kc_in;

    return kc_in;
}


// when someone calls KernelContextSwitch, it might come to here.
KernelContext *MyTrueKCS(KernelContext *kc_in,void *curr,void *next){
    TracePrintf(1, "Enter MyTrueKCS\n");

    int i, stackInx = 0;

    lstnode* curr_pcb_node = (lstnode*) curr;
    lstnode* next_pcb_node = (lstnode*) next;
    
    pcb_t *cur_p = TurnNodeToPCB(curr_pcb_node);
    pcb_t *next_p = TurnNodeToPCB(next_pcb_node);

    //Copy the kernel context to current process's pcb
    cur_p->kctxt = *kc_in;

    //Remember to change page table entries for kernel stack
    for (i = g_kStackStPage; i <= g_kStackEdPage; i++){
        g_pageTableR0[i] = next_p->krnlStackPtb[stackInx];
        // TracePrintf(1, "next_p->krnlStackPtb[stackInx].pfn:%d\n",next_p->krnlStackPtb[stackInx].pfn);
        // TracePrintf(1, "g_pageTableR0[i].pfn:%d\n",g_pageTableR0[i].pfn);
        stackInx++;
    }

    //=============================================
    // Use a safety margin page as a buffer of copying memory
    g_pageTableR0[SAFETY_MARGIN_PAGE].valid = 1;
    g_pageTableR0[SAFETY_MARGIN_PAGE].prot = (PROT_READ | PROT_WRITE);

    for (i = 0; i < g_pageNumOfStack; i++){
        g_pageTableR0[SAFETY_MARGIN_PAGE].pfn = next_p->krnlStackPtb[i].pfn;

        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

        unsigned int desAddr = SAFETY_MARGIN_PAGE << PAGESHIFT;
        unsigned int srcAddr = (g_kStackStPage + i) << PAGESHIFT;

        memcpy((void *)desAddr, (void *)srcAddr, PAGESIZE);
    }

    //Restore the buffer.
    g_pageTableR0[SAFETY_MARGIN_PAGE].valid = 0;
    g_pageTableR0[SAFETY_MARGIN_PAGE].prot = PROT_NONE;
    g_pageTableR0[SAFETY_MARGIN_PAGE].pfn = UNALLOCATED;

    //Write Page table of current process into register
    WriteRegister(REG_PTBR1, (unsigned int) next_p->usrPtb);
    WriteRegister(REG_PTLR1, (unsigned int) MAX_PT_LEN);
    //=============================================
    
    //Flush All TLB because 1. Kernel Stack Mapping has changed 2. User Page Table has been written into register
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

    //Turn Next Process to be the current one
    currProc = next_pcb_node; 

    TracePrintf(1, "currProc->pc:%p\n", TurnNodeToPCB(currProc)->uctxt.pc);
    //Return a pointer to a kernel context it had earlier saved
    return &(next_p->kctxt);

}

