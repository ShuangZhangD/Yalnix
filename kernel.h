#include "hardware.h"
#include "datastructure.h"

typedef struct pte pte_t;

//global variables
pte_t g_pageTableR0[MAX_PT_LEN];
dblist* g_freeFrame;

void SetKernelData(void *_KernelDataStart ,void *_KernelDataEnd);

void KernelStart(char *cnd_args[],unsigned int pmem_size, UserContext *uctxt);

int SetKernelBrk(void *addr);

void DoIdle(void);