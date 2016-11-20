#include "hardware.h"
#include "selfdefinedstructure.h"

pte_t g_pageTableR0[MAX_PT_LEN];  //Kerenl Page Table

/*
	Get KerenlBrk, Kerenl Data Start Address, Kernel Data End Address
*/
void SetKernelData(void *_KernelDataStart ,void *_KernelDataEnd);
/*
	1. Initialize Interrupt Table
	2. Initialize Free Frame Tracking
	3. Initialize User/Kerenl Page Table
	4. Enable Virtual Memory
	5. Initialize Queues
	6. Initialize Idle Process and Init Process (could load program into the InitProcess)
*/
void KernelStart(char *cmd_args[],unsigned int pmem_size, UserContext *uctxt);

/*
	1. Check Page Status
	2. Map or unmap frame to kernel brk
	3. Set new Kernel break
*/
int SetKernelBrk(void *addr);


// Initialize  an Idle Processs, it will run DoIdle in Yalnix
pcb_t *InitIdleProc(UserContext *uctxt);  

/*
	Initialize a Process, includingL
	1. Initialize PCB
	2. Initialize User Page Table
	3. Initialize frame mapping to Kernel Stack
	4. Miscellaneous Initialization (pid, process state, etc)
*/
lstnode *InitProc();

//Load DoIdle into Idle Process
void CookDoIdle(UserContext *uctxt);

// An infinite while loop printing DoIdle
void DoIdle(void);

// Use Doubly Linkedlist to track free frames
void InitFreeFrameTracking(int pmem_size);

// Initialize User Page Table
pte_t* InitUserPageTable ();

// Initialize Kernel Page Table
void InitKernelPageTable();

// Use a SAFETY_MARGIN_PAGE to copy kernel stack for Processes in Fork or KernelStart to have same kernel variables(Local Variables, etc)
void CopyKernelStack (pte_t* pageTable);

/*
	Where Scheduling Happens:
	1. Copy Kernel Context
	2. Change the mapping of Kernel Stack
	3. Change Page Table
	4. Assign the first node the ready queue to be the current process!
*/
KernelContext *MyTrueKCS(KernelContext *kc_in,void *curNode,void *nxtNode);

/* 
	For cloning KernelContext, it does not really doing switch:
	1.But it copies kernel stack from one process to another process
*/
KernelContext *MyCloneKCS(KernelContext *kc_in,void *curNode,void *nxtNode);

/*
	For Terminating A Process, here are some extra tasks it does:
	1. Doing some free memory stuff
	2. Dealing with terminated child which could be reaped by Syscall--Wait(int *status_ptr);
*/
KernelContext *MyTerminateKCS(KernelContext *kc_in,void *termNode,void *nxtNode);

/*
	For Fork 
	1. It copies things in kernel stack
	2. However, it does not change the mapping of kernel stack
*/
KernelContext *MyForkKCS(KernelContext *kc_in,void *curNode,void *nxtNode);

/*
	It seems OBSELETE now......
*/
KernelContext *MyIOKCS(KernelContext *kc_in,void *curNode,void *nxtNode);


//Get Mutex Id
int getMutexId();

/*
	1. Check whether addr is valid
	2. Make sure VMEM_BASE to addr(but not including) are valid
	3. Make sure those beyond this region are invalid (except for kernel stack)
*/
int CheckPageStatus(unsigned int addr);

/*
	Syscall
*/
int KernelReclaim(UserContext *uctxt);

