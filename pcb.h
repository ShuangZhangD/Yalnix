#include "yalnix.h"
#include "hardware.h"
#include "selfdefinedstructure.h"
#include "listcontrol.h"

dblist* waitingqueue;
dblist* readyqueue;
dblist* terminatedqueue;

enum processState {RUNNING,WAITING,READY,TERMINATED}; //define the state of a process

int traverseParent(pcb_t *proc);              //Traverse through its parent
int traverseChildren(pcb_t *proc);            //Traverse through its 
int apppendProcess(pcb_t *des, pcb_t *src);     //Add a process in PCB list
int removeProcessBypid(pcb_t *des, int pid);   //Remove a process from PCB by PID
int findProcessBypid(pcb_t *des, int pid);      //Find a process by PID
void terminateProcess(pcb_t *proc);
int enreadyqueue(pcb_t* proc,dblist* readyqueue);
void* dereadyqueue(pcb_t* proc,dblist* readyqueue);
int enwaitingqueue(pcb_t* proc,dblist* waitingqueue);
void* dewaitingqueue(pcb_t* pcb,dblist* waitingqueue);
//Temp
void terminateProcess(pcb_t *proc);
int GrowUserStack(pcb_t *proc, unsigned int addr);
