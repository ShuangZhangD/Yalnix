#include "yalnix.h"
#include "hardware.h"

enum processState {RUNNING,WAITING,BLOCKED,TERMINATED}; //define the state of a process

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

int traverseParent(pcb_t *proc);              //Traverse through its parent
int traverseChildren(pcb_t *proc);            //Traverse through its 
int apppendProcess(pcb_t *des, pcb_t *src);     //Add a process in PCB list
int removeProcessBypid(pcb_t *des, int pid);   //Remove a process from PCB by PID
int findProcessBypid(pcb_t *des, int pid);      //Find a process by PID


//Temp
void terminateProcess(pcb_t *proc);
int GrowUserStack(pcb_t *proc, unsigned int addr);
int checkAvailFrame(int fn);