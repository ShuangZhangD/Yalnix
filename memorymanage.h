#include "selfdefinedstructure.h"
#include "processmanage.h"

int KernelBrk(UserContext *uctxt);

void TrapMemory(UserContext *uctxt);

void emptyregion1pagetable(pcb_t *proc);

int WritePageTable(pte_t *pagetable, int startPage, int endPage, int valid, int prot);

void ummap(pte_t *pagetable, int startPage, int endPage, int valid, int prot);

int GrowUserStack(lstnode *procnode, int addrPage);

void* MallocCheck(int size);

int InputSanityCheck(int *addr);