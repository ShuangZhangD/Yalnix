#include "hardware.h"


typedef struct pte pte_t;

//global variables
unsigned int m_kernel_brk;
unsigned int m_kernel_data_start;
struct pte g_pageTableR0[MAX_PT_LEN];


int kernelfork(UserContext *uctxt);

int kernelexec(UserContext *uctxt);

int kernelexit(UserContext *uctxt);

int kernelwait(UserContext *uctxt);

int kernelgetpid(UserContext *uctxt);

int kernelbrk(UserContext *uctxt);

int kerneldelay(UserContext *uctxt);

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

void KernelStart(char *cnd_args[],unsigned int pmem_size, UserContext *uctxt);

int SetKernelBrk(void *addr);

void DoIdle(void);

