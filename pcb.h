#include"hardware.h"
#include"yalnix.h"

enum processState {WAITING, RUNNING,BLOCKED,TERMINATED}; //define the state of a process

typedef struct ProcessControlBlock{
    int processState;                       //State of Process   
    int pid;                                //Process ID
    UserContext uctxt;                      //Snapshot of user context
    KernelContext kctxt;                    //Snapshot of kernel context
    struct pte *pageTable;                  //PageTable

    PCB *parent;                            //A pointer to parent process
    dlqueue *children;                        //A pointer to mulitple child processes

} PCB;


int traverseParent(PCB *proc);              //Traverse through its parent
int traverseChildren(PCB *proc);            //Traverse through its 
int apppendProcess(PCB *des, PCB *src);     //Add a process in PCB list
int removeProcessBypid(PCB *des, int pid);   //Remove a process from PCB by PID
int findProcessBypid(PCB *des, in pid);      //Find a process by PID


