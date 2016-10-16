#include "hardware.h"
#include "yalnix.h"
// #include "kernel.h"

enum processState {WAITING, RUNNING,BLOCKED,TERMINATED}; //define the state of a process

typedef struct ProcessControlBlock{
    int processState;                       //State of Process   
    int pid;                                //Process ID
    UserContext *uctxt;                      //Snapshot of user context
    KernelContext *kctxt;                    //Snapshot of kernel context
    pte_t usrPtb[MAX_PT_LEN];               //PageTable for Userland
    pte_t *krnlPtb;                         //Pointer to Kernel Page Table
    long krnlPtbSize;
    pte_t *parent;                            //A pointer to parent process
    dlqueue *children;                        //A pointer to mulitple child processes

} pcb_t;


int traverseParent(pcb_t *proc);              //Traverse through its parent
int traverseChildren(pcb_t *proc);            //Traverse through its 
int apppendProcess(pcb_t *des, pcb_t *src);     //Add a process in PCB list
int removeProcessBypid(pcb_t *des, int pid);   //Remove a process from PCB by PID
int findProcessBypid(pcb_t *des, int pid);      //Find a process by PID


