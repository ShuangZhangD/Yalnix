#ifndef DS_H_INCLUDED
#define DS_H_INCLUDED

#include "hardware.h"
#include "yalnix.h"

#define VALID 1
#define INVALID 0
#define KILL (-2)
#define UNALLOCATED (-1)
#define SUCCESS 0
#define SAFETY_MARGIN_PAGE (KERNEL_STACK_LIMIT-KERNEL_STACK_MAXSIZE-PAGESIZE) >> PAGESHIFT

enum processState {WAITING,READY,TERMINATED}; //define the state of a process

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
    int procState;                          //State of Process   
    int pid;                                //Process ID
    UserContext uctxt;                      //Snapshot of user context
    KernelContext kctxt;                    //Snapshot of kernel context
    pte_t *usrPtb;                          //PageTable for Userland
    pte_t *krnlStackPtb;                    //Pointer to Kernel Page Table
    int krnlStackPtbSize;                   //Size of kernel stack page table
    lstnode *parent;                        //A pointer to parent process
    dblist *children;                       //A pointer to mulitple child processes
    dblist *terminatedchild;                //A pointer to terminated child 
    int stack_limit_page;                   //Lowest page of user stack 
    int brk_page;                           //Break page
    int data_page;                          //Data page
    int clock;                              //Clock Count Down for syscall:Delay();
    int exitstatus;                         //Exit Status

} pcb_t;

typedef struct Terminal{
    dblist* readerwaiting;
    dblist* writerwaiting;
    dblist* bufferqueue;
    int LeftBufLen;
    char transmitbuf[TERMINAL_MAX_LINE];
    char receivebuf[TERMINAL_MAX_LINE];

} Tty;

typedef struct Message{
    char buf[TERMINAL_MAX_LINE];
    int len;
} msg_t;


typedef struct pipe
{
    int id;
    int len;
    int contentlen;
    char buffer[PIPE_BUFFER_LEN];
    dblist* readers;
} pipe_t;

typedef struct lock{
    int lock_id;
    int ownerid;
    dblist *waitlist;
} lock_t;

typedef struct cvar{
    int cvar_id;
    dblist* cvarwaiting;
} cvar_t;

typedef struct semaphore{
    int sem_id;
    int sem_val;
    dblist* semwaitlist;
} sem_t;


typedef void (*trapvector_t) (UserContext*);
#endif 