#include "selfdefinedstructure.h"

void emptyregion1pagetable(pcb_t *proc);

void writepagetable(pte_t *pagetable, int startPage, int endPage, int valid, int prot);

int GrowUserStack(lstnode *procnode, unsigned int addr);