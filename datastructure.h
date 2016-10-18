#ifndef DS_H_INCLUDED
#define DS_H_INCLUDED

#include "hardware.h"

typedef struct node{
 	int id;
 	int used;
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
    UserContext *uctxt;                      //Snapshot of user context
    KernelContext *kctxt;                    //Snapshot of kernel context
    pte_t usrPtb[MAX_PT_LEN];               //PageTable for Userland
    pte_t *krnlStackPtb;                         //Pointer to Kernel Page Table
    int krnlStackPtbSize;
    pte_t *parent;                            //A pointer to parent process
    dblist *children;                        //A pointer to mulitple child processes
    unsigned int sp; //TODO temp
    unsigned int brk; //TODO temp
} pcb_t;


typedef void (*trapvector_t) (UserContext*);
#endif 