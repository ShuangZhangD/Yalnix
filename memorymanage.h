#include "selfdefinedstructure.h"
#include "processmanage.h"

/*Trap & Syscalls*/
int KernelBrk(UserContext *uctxt);

void TrapMemory(UserContext *uctxt);
/*Trap & Syscalls*/


//Unmap Page Table and recycle free frame
int EmptyRegion1PageTable(pcb_t *proc);

//Write From startPage to endPage
int WritePageTable(pte_t *pagetable, int startPage, int endPage, int valid, int prot);

/* 
	1. Rewrite protection, validity and pfn. 
	2. recycle Frame!
*/
int Unmap(pte_t *pagetable, int startPage, int endPage, int valid, int prot);

/*
	1. Check whether addrPage is a valid page
	2. WritePageTable, assign new frames
	3. Assign the new value to the process
*/
int GrowUserStack(lstnode *procnode, int addrPage);

/*
	1. Malloc
	2. Zero out all the memories	
*/
void* MallocCheck(int size);

/*
	1. Ensure addr will not touch Region 1 or beyond the limit of Virtual Memory
*/
int InputSanityCheck(int *addr);