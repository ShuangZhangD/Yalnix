#include "kernel.h"
#include "yalnix.h"
#include "listcontrol.h"
#include "processmanage.h"
#include "loadprogram.h"
#include "io.h"

//Global Variables
int m_enableVM = 0;                                                     //A flag to check whether Virtual Memory is enabled(1:enabled, 0:not enabled)
int g_pid = 1;                                                          //PID                             
int g_mutex_id = 0;                                                     //ID for CVar, Lock, Pipe, Semaphores.....
// int g_isInitial = 1;
int const g_pageNumOfStack = KERNEL_STACK_MAXSIZE / PAGESIZE;           //Number of Pages of Kernel Stack
int const g_kStackStPage = KERNEL_STACK_BASE >> PAGESHIFT;              //Kerenl Stack Start Page
int const g_kStackEdPage = (KERNEL_STACK_LIMIT - 1) >> PAGESHIFT;       //Kerenl Stack End age

unsigned int m_kernel_brk;                                              //Page of Kernel Brk
unsigned int m_kernel_data_start;                                       //Page of Kernel Data Start Page
unsigned int m_kernel_data_end;                                         //page of Kernel Data End Page


lstnode* currProc;                                                      //Current Process, not belonging to any queue, we will put it into queue only before or in the Context Switch
dblist* freeframe_list;                                                 //FreeFrameList

extern dblist* waitingqueue;                                            //Queue for Blocked Process (due to delay)
extern dblist* readyqueue;                                              //Queue for ready to be scheduled processes
extern dblist* lockqueue;                                               //Queue for storing different Lock
extern dblist* cvarqueue;                                               //Queue for storing different Cvar
extern dblist* pipequeue;                                               //Queue for storing different pipes
extern dblist* semqueue;                                                //Queue for storing different semaphores
extern Tty* tty[NUM_TERMINALS];                                         //TTy Terminals for Input / Output 

/*
   Kernel Initiailization functions =======================================================================
 */
void SetKernelData(void *_KernelDataStart, void *_KernelDataEnd){
    TracePrintf(2, "_KernelDataStart: %p\n", _KernelDataStart);

    //Save the addresses of initial break and data segment in global variables
    m_kernel_brk = (unsigned int) _KernelDataEnd;
    m_kernel_data_end = (unsigned int) _KernelDataEnd;
    m_kernel_data_start = (unsigned int) _KernelDataStart;

    TracePrintf(3, "KernelDataStart = %p \n", m_kernel_data_start);
    TracePrintf(3, "KernelDataEnd = %p \n", m_kernel_brk);  
    return;
}

int SetKernelBrk(void *addr){
    TracePrintf(2, "Enter SetKernelBrk! addr = %x, m_enableVM = %d\n", addr, m_enableVM);

    int i,rc = 0;
    unsigned int newBrk = (unsigned int) addr;
    int newBrkPage = newBrk >> PAGESHIFT;
    int oldBrkPage = m_kernel_brk >> PAGESHIFT;

    if (m_enableVM){
        rc = CheckPageStatus(newBrk);
        if (rc) {
            return ERROR;
        }

        if (newBrkPage > oldBrkPage){
            rc = WritePageTable(g_pageTableR0, oldBrkPage, newBrkPage - 1, VALID, (PROT_READ | PROT_WRITE));
            if (rc){
                return ERROR;
            }
            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0); //Good Boy should remember to FLUSH it. :)
        }else if (newBrkPage < oldBrkPage){
            rc = Unmap(g_pageTableR0, newBrkPage, oldBrkPage, INVALID, PROT_NONE);
            if (rc){
                return ERROR;
            }         
            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0); //Good Boy should remember to FLUSH it. :)
        }
    }

    //Let addr be the new kernel break
    m_kernel_brk = newBrk;
    TracePrintf(2, "Exit SetKernelBrk! New Brk! m_kernel_brk = %p\n", m_kernel_brk);
    return SUCCESS;
}


void KernelStart(char *cmd_args[],unsigned int pmem_size, UserContext *uctxt){
    int i, rc, stackInx = 0;
    //Initialize interrupt vector table and REG_VECTOR_BASE
    TracePrintf(3, "Init interrupt table.\n");
    InitInterruptTable();

    //Build a structure to track free frame
    TracePrintf(3, "Init free frame list.\n");
    InitFreeFrameTracking(pmem_size);

    //Build initial page table for Region 1 (before kernel page protection)
    TracePrintf(3, "Init user page table \n");
    pte_t *idlePageTable = InitUserPageTable();

    // //Build initial page table for Region 0
    TracePrintf(3, "Init kernel page table \n");
    InitKernelPageTable();

    WriteRegister(REG_PTBR0, (unsigned int) g_pageTableR0);
    WriteRegister(REG_PTLR0, (unsigned int) MAX_PT_LEN);

    WriteRegister(REG_PTBR1, (unsigned int) idlePageTable);
    WriteRegister(REG_PTLR1, (unsigned int) MAX_PT_LEN);

    TracePrintf(3, "Enable VM!\n");
    WriteRegister(REG_VM_ENABLE,1);
    m_enableVM = 1;

    //init Queue
    waitingqueue = listinit();
    readyqueue = listinit();
    lockqueue = listinit();
    cvarqueue = listinit();
    pipequeue = listinit();
    blockqueue = listinit();
    semqueue = listinit();

    for (i = 0; i < NUM_TERMINALS; i++)
    {
        tty[i] = (Tty*) MallocCheck(sizeof(Tty));
        if (NULL == tty[i]){
            TracePrintf(1, "Malloc Failed in KernelStart! TTY is NULL!\n");
            return;
        }
        tty[i]->readerwaiting = listinit();
        tty[i]->writerwaiting = listinit();
        tty[i]->bufferqueue = listinit();
        tty[i]->LeftBufLen = 0;
    }

    //Initialize Idle Process    
    TracePrintf(3, "Init pcb.\n");
    pcb_t *idlePcb = InitIdleProc(uctxt);
    if (NULL == idlePcb){
        TracePrintf(1, "Error! IdlePcb == NULL\n");
        return;
    }
    idlePcb->usrPtb = idlePageTable;
    idlePcb->usrPtb[MAX_PT_LEN-1].valid = 1;
    idlePcb->usrPtb[MAX_PT_LEN-1].prot = (PROT_WRITE | PROT_READ);
    idlePcb->usrPtb[MAX_PT_LEN-1].pfn = remove_head(freeframe_list)->id;

    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

    //====Cook DoIdle()====
    TracePrintf(3, "Cook DoIdle\n");
    CookDoIdle(uctxt);

    idlePcb->uctxt = *uctxt;
    lstnode *idleProc = TurnPCBToNode(idlePcb);

    //Initialize Init Process
    lstnode *initProc = InitProc();
    if (NULL == initProc){
        TracePrintf(1, "Error! initProc == NULL\n");
        return;
    }

    //Create first process  and load initial program to it
    if (NULL == cmd_args[0]){
        rc = LoadProgram("init", cmd_args, initProc);
        TracePrintf(1, "Init Proc, cmd_args:%s\n", *cmd_args);
    } else {
        rc = LoadProgram(cmd_args[0], cmd_args, initProc); 
    }
    if (rc){
        Halt();
        return;
    }

    //Idle Process is ready for Context Switch!
    enreadyqueue(idleProc, readyqueue);

    //Make Init Process the Current Process
    currProc = initProc;

    TracePrintf(3,"idle usrPtb:%p, init usrPtb:%p\n", idlePcb->usrPtb, TurnNodeToPCB(initProc)->usrPtb);

    //Clone Kernel Context and copy kernel stack and change kernel frame mapping
    rc = KernelContextSwitch(MyCloneKCS, (void *)idleProc, (void *)initProc);
    if (rc) {
        TracePrintf(1, "Context Switch in KernelStart goes wrong.\n");
    }

    //Put it here because after context switch, a new current process should now where to go
    *uctxt = TurnNodeToPCB(currProc)->uctxt;

    TracePrintf(2,"Exit KernelStart.\n");
    return;
}   

//  =========================================================================

pte_t* InitUserPageTable (){
    int i;

    pte_t *usrPtb = (pte_t *) MallocCheck(sizeof(pte_t) * MAX_PT_LEN);
    if (NULL == usrPtb){
        TracePrintf(1, "Malloc Failed! InitUserPageTable get a NULL page!\n");
        return NULL;
    }

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
        TracePrintf(2, "DoIdle\n");
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
    TracePrintf(2, "Enter InitProc\n");

    int i, stackInx;

    //Initialize Process
    pcb_t *proc = (pcb_t *) MallocCheck (sizeof(pcb_t));
    if (NULL == proc){
        TracePrintf(1, "Malloc Failed! Get a NULL proc in InitProc!\n");
        return NULL;
    }

    //Initialize User Page Table
    proc->usrPtb = InitUserPageTable();
    if (NULL == proc->usrPtb){
        return NULL;
    }

    //Initialize Kernel Stack Page Table
    proc->krnlStackPtb = (pte_t *) MallocCheck(g_pageNumOfStack * sizeof(pte_t));
    if (NULL == proc->krnlStackPtb){
        TracePrintf(1, "Malloc Failed! Get a NULL krnlStackPtb in InitProc!\n");  
        return NULL;     
    }

    proc->krnlStackPtbSize = g_pageNumOfStack;

    //Let a userprocess have its own kernel stack
    for (i=g_kStackStPage, stackInx = 0; i<=g_kStackEdPage; i++, stackInx++){
        proc->krnlStackPtb[stackInx].prot = g_pageTableR0[i].prot;
        proc->krnlStackPtb[stackInx].valid = g_pageTableR0[i].valid;

        lstnode *first = remove_head(freeframe_list);
        proc->krnlStackPtb[stackInx].pfn = first->id;
    }

    //Initializing variables
    proc->procState = READY;
    proc->pid = g_pid++;

    proc->children = listinit();
    proc->terminatedchild = listinit();

    TracePrintf(2, "Exit InitProc\n");
    return TurnPCBToNode(proc);
}

pcb_t *InitIdleProc(UserContext *uctxt){
    int i, stackInx;

    //Initialize Process
    pcb_t *proc = (pcb_t *) MallocCheck(sizeof(pcb_t));
    if (NULL == proc){
        TracePrintf(1, "Malloc Failed! Get a NULL proc in InitIdleProc!\n");
        return NULL;       
    }

    //Initialize Kernel Stack Page Table
    proc->krnlStackPtb = (pte_t *) MallocCheck(g_pageNumOfStack * sizeof(pte_t));
    if (NULL == proc->krnlStackPtb){
        TracePrintf(1, "Malloc Failed! Get a NULL krnlStackPtb in InitIdleProc!\n");
        return NULL;       
    }
    proc->krnlStackPtbSize = g_pageNumOfStack;

    //Let a userprocess have its own kernel stack
    for (i=g_kStackStPage, stackInx = 0; i<=g_kStackEdPage; i++, stackInx++){
        proc->krnlStackPtb[stackInx].pfn = g_pageTableR0[i].pfn;
        proc->krnlStackPtb[stackInx].prot = g_pageTableR0[i].prot;
        proc->krnlStackPtb[stackInx].valid = g_pageTableR0[i].valid;
    }

    //Initializing variables
    proc->procState = READY;
    proc->pid = g_pid++;
    proc->uctxt = *uctxt;

    return proc;
}


int CheckPageStatus(unsigned int addr){
    unsigned int i, pageAddr = (addr >> PAGESHIFT),rc = 0;

    //Check whether the addr is a valid address
    if (addr < m_kernel_data_end) {
        TracePrintf(1, "Error! Address is lower than kernel_data_end!\n");
        return ERROR;
    }

    //make sure addresses from [addr] (but not including) to [VMEM_BASE] are valid
    for (i = (VMEM_BASE >> PAGESHIFT); i < pageAddr; i++){
        // if (0 == g_pageTableR0[i].valid) return ERROR;
        g_pageTableR0[i].valid = VALID;
    }

    //check no virtual memory in Region 0 out of range are valid
    for (i = pageAddr; i <= SAFETY_MARGIN_PAGE; i++){
        if (VALID == g_pageTableR0[i].valid) {
            TracePrintf(1, "Error! page:%d should not be valid, but it's valid!\n",i);
            return ERROR;
        }
        g_pageTableR0[i].valid = INVALID;
    }

    return SUCCESS;
}

void CopyKernelStack (pte_t* pageTable){
    TracePrintf(2, "Enter CopyKernelStack\n");

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

    TracePrintf(2, "Exit CopyKernelStack\n");
    return;
}

KernelContext *MyCloneKCS(KernelContext *kc_in,void *curNode,void *nxtNode){
    TracePrintf(2, "Enter MyCloneKCS\n");

    int i, stackInx = 0;

    //Turn Node to PCB will make things easier
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

    TracePrintf(2, "Exit MyCloneKCS\n");
    return kc_in;
}


// when someone calls KernelContextSwitch, it might come to here.
KernelContext *MyTrueKCS(KernelContext *kc_in,void *curNode,void *nxtNode){
    TracePrintf(2,"Enter MyTrueKCS\n");

    int i ,stackInx = 0;

    //Turn Node to PCB
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

    //
    lstnode* node = dereadyqueue(readyqueue);
    if (node != next_pcb_node) TracePrintf(1, "Error in MyTrueKCS!");
    currProc = node; 

    TracePrintf(2,"Exit MyTrueKCS\n");
    //Return a pointer to a kernel context it had earlier saved
    return &(next_p->kctxt);
}

KernelContext *MyTerminateKCS(KernelContext *kc_in,void *termNode,void *nxtNode){
    TracePrintf(2,"Enter MyTerminateKCS\n");

    int i, stackInx = 0;

    //Turn Node to PCB
    lstnode* term_pcb_node = (lstnode*) termNode;
    lstnode* next_pcb_node = (lstnode*) nxtNode;

    pcb_t *term_p = TurnNodeToPCB(term_pcb_node);
    pcb_t *next_p = TurnNodeToPCB(next_pcb_node);

    //Remember to change page table entries for kernel stack
    for (i = g_kStackStPage; i <= g_kStackEdPage; i++){
        g_pageTableR0[i].pfn = next_p->krnlStackPtb[stackInx].pfn;
        stackInx++;
    }

    WriteRegister(REG_PTBR1, (unsigned int) next_p->usrPtb);
    WriteRegister(REG_PTLR1, (unsigned int) MAX_PT_LEN);
    //Flush All TLB because 1. Kernel Stack Mapping has changed
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

    //Recycle Free Frame
    EmptyRegion1PageTable(term_p);

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
            //When parent is blocked by syscall--Wait(), we could pull parent out here. :)
            lstnode* node = deblockqueue(term_p->parent,blockqueue);
            TracePrintf(3,"term_p->parent->id:%d\n", term_p->parent->id);
            TracePrintf(3,"node->id:%d\n", node->id);
            enreadyqueue(node,readyqueue);       
        }

        //From Child to DeadChild
        lstnode* node = remove_node(term_p->pid, currParent->children);

        free(term_p->usrPtb);
        free(term_p->krnlStackPtb); 
        free(term_p->children);
        free(term_p->terminatedchild);

        insert_tail(node,currParent->terminatedchild);
    } 
    //Recycle Memory
    free(term_pcb_node);

    lstnode* node = dereadyqueue(readyqueue);
    if (node != next_pcb_node) TracePrintf(1, "KernelContextSwitch Error!");
    currProc = node;

    TracePrintf(2,"Exit MyTerminateKCS\n");

    //Return a pointer to a kernel context it had earlier saved
    return &(next_p->kctxt);

}


KernelContext *MyForkKCS(KernelContext *kc_in,void *curNode,void *nxtNode){
    TracePrintf(2, "Enter MyTestKCS\n");

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

    TracePrintf(2, "Exit MyTestKCS\n");
    return kc_in;
}

int getMutexId(){
    return ++g_mutex_id;
}

/*
    Syscall
*/
int KernelReclaim(UserContext *uctxt)
{
    int id = uctxt->regs[0];

    //Pipe
    if (search_node(id , pipequeue)!= NULL)
    {
        lstnode* pipenode = remove_node(id , pipequeue);
        pipe_t* pipe = pipenode->content;
        if(pipe->readers != NULL)
        {
            free(pipe->readers);
        }    
        free(pipe);
        free(pipenode);

        return SUCCESS;
    } 

    //Lock
    if (search_node(id , lockqueue) != NULL)
    {
        int id = uctxt->regs[0];
        lstnode* locknode = remove_node(id , lockqueue);
        lock_t* lock = locknode->content;

        if(lock->waitlist != NULL)
        {
            free(lock->waitlist);
        }
        free(lock);
        free(locknode); 

        return SUCCESS;          
    } 

    //Cvar
    if (search_node(id , cvarqueue) != NULL)
    {
        int id = uctxt->regs[0];
        lstnode* cvarnode = remove_node(id , cvarqueue);
        cvar_t* cvar = cvarnode->content;
        if(cvar->cvarwaiting != NULL){
            free(cvar->cvarwaiting);
        }
        free(cvar);
        free(cvarnode);

        return SUCCESS;

    }

    //Semaphore
    if (search_node(id , semqueue) != NULL)
    {
        int id = uctxt->regs[0];
        lstnode* semnode = remove_node(id , semqueue);
        sem_t* sem = semnode->content;
        if(sem->semwaitlist != NULL){
            free(sem->semwaitlist);
        }
        free(sem);
        free(semnode);

        return SUCCESS;
        
    }

    return ERROR;
}

//OBSELETE.......=====================================================================

KernelContext *MyIOKCS(KernelContext *kc_in,void *curNode,void *nxtNode){
    TracePrintf(2,"Enter MyIOKCS\n");

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

    TracePrintf(2,"Exit MyIOKCS\n");
    //Return a pointer to a kernel context it had earlier saved
    return &(next_p->kctxt);
}


