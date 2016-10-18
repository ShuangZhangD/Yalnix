#include "yalnix.h"
#include "hardware.h"
#include "datastructure.h"

enum processState {RUNNING,WAITING,BLOCKED,TERMINATED}; //define the state of a process

int traverseParent(pcb_t *proc);              //Traverse through its parent
int traverseChildren(pcb_t *proc);            //Traverse through its 
int apppendProcess(pcb_t *des, pcb_t *src);     //Add a process in PCB list
int removeProcessBypid(pcb_t *des, int pid);   //Remove a process from PCB by PID
int findProcessBypid(pcb_t *des, int pid);      //Find a process by PID


//Temp
void terminateProcess(pcb_t *proc);
int GrowUserStack(pcb_t *proc, unsigned int addr);
int checkAvailFrame(int fn);