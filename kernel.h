#include "hardware.h"
#include "selfdefinedstructure.h"

//global variables
unsigned int m_kernel_brk;
unsigned int m_kernel_data_start;
unsigned int m_kernel_data_end;

pte_t g_pageTableR0[MAX_PT_LEN];

pcb_t *idleProc;
dblist* freeFrame_list;

pcb_t *InitIdleProc(UserContext *uctxt);

lstnode *InitProc();

int kernelregister(UserContext *uctxt);

int kernelsend(UserContext *uctxt);

int kernelreceive(UserContext *uctxt);

int kernelreceivespecific(UserContext *uctxt);

int kernelreply(UserContext *uctxt);

int kernelforward(UserContext *uctxt);

int kernelcopyfrom(UserContext *uctxt);

int kernelcopyto(UserContext *uctxt);

int kernelreadsector(UserContext *uctxt);

int kernelwritesector(UserContext *uctxt);

int kernelreclaim(int id);

void SetKernelData(void *_KernelDataStart ,void *_KernelDataEnd);

void KernelStart(char *cmd_args[],unsigned int pmem_size, UserContext *uctxt);

int SetKernelBrk(void *addr);

void DoIdle(void);

void InitFreeFrameTracking(int pmem_size);

void CookDoIdle(UserContext *uctxt);

pte_t* InitUserPageTable ();

void InitKernelPageTable();

void CopyKernelStack (pte_t* pageTable);

KernelContext *MyCloneKCS(KernelContext *kc_in,void *curNode,void *nxtNode);

KernelContext *MyTrueKCS(KernelContext *kc_in,void *curNode,void *nxtNode);

KernelContext *MyIOKCS(KernelContext *kc_in,void *curNode,void *nxtNode);

KernelContext *MyTerminateKCS(KernelContext *kc_in,void *termNode,void *nxtNode);

KernelContext *MyForkKCS(KernelContext *kc_in,void *curNode,void *nxtNode);

