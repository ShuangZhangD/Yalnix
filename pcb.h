#include "yalnix.h"
#include "hardware.h"
#include "selfdefinedstructure.h"
#include "listcontrol.h"
#include "kernel.h"

dblist* waitingqueue;
dblist* readyqueue;
dblist* terminatedqueue;

enum processState {RUNNING,WAITING,READY,TERMINATED}; //define the state of a process

int switchproc();
int traverseParent(lstnode *procnode);              //Traverse through its parent
int traverseChildren(lstnode *procnode);            //Traverse through its 
int apppendProcess(lstnode *procnode, pcb_t *src);     //Add a process in PCB list
int removeProcessBypid(pcb_t *des, int pid);   //Remove a process from PCB by PID
int findProcessBypid(pcb_t *des, int pid);      //Find a process by PID

int enreadyqueue(lstnode *procnode,dblist* queue);
void* dereadyqueue(dblist* queue);
int enwaitingqueue(lstnode *procnode,dblist* queue);
void* dewaitingqueue(lstnode* waitingnode,dblist* queue);

lstnode* TurnPCBToNode(pcb_t *pcb);
pcb_t* TurnNodeToPCB(lstnode *node);

//Temp
void terminateProcess(lstnode *procnode);