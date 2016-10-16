#include"hardware.h"

//global variables
int m_enableVM = 0; //A flag to check whether Virtual Memory is enabled(1:enabled, 0:not enabled)
int m_kernel_brk;
int m_kernel_data_start;
pte g_pageTableR0[MAX_PT_LEN];

dblist* g_freeFrame;

void SetKernelData(void *_KernelDataStart ,void *_KernelDataEnd);

void KernelStart(char *cnd_args[],unsigned int pmem_size, UserContext *uctxt);

int SetKernelBrk(void *addr);
