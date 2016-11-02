#ifndef DS_H_INCLUDED
#define DS_H_INCLUDED

#include "hardware.h"

#define VALID 1
#define INVALID 0
#define KILL (-2)
#define UNALLOCATED (-1)
#define SUCCESS 0
#define SAFETY_MARGIN_PAGE (KERNEL_STACK_LIMIT-KERNEL_STACK_MAXSIZE-PAGESIZE) >> PAGESHIFT

typedef struct node{
 	int id;
 	void* content;
 	struct node *pre;
	struct node *next;
} lstnode;

typedef struct lst
{
	int size;
	lstnode *head;
	lstnode *tail;
}dblist;

typedef struct pte pte_t;

typedef struct ProcessControlBlock{
    int procState;                       //State of Process   
    int pid;                                //Process ID
    UserContext uctxt;                      //Snapshot of user context
    KernelContext kctxt;                    //Snapshot of kernel context
    pte_t *usrPtb;               //PageTable for Userland
    pte_t *krnlStackPtb;                         //Pointer to Kernel Page Table
    int krnlStackPtbSize;
    lstnode *parent;                            //A pointer to parent process
    dblist *children;                        //A pointer to mulitple child processes
    dblist* terminatedchild;
    int stack_limit;
    int brk; 
    int clock;
    int exitstatus;
} pcb_t;

typedef void (*trapvector_t) (UserContext*);
#endif 