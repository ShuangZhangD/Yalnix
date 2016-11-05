#include "yalnix.h"
#include "hardware.h"
#include "selfdefinedstructure.h"
#include "listcontrol.h"
#include "kernel.h"

dblist* waitingqueue;
dblist* readyqueue;
dblist* terminatedqueue;

int kernelfork(UserContext *uctxt);

int kernelexec(UserContext *uctxt);

int kernelexit(UserContext *uctxt);

int kernelwait(UserContext *uctxt);

int kernelgetpid();

int kerneldelay(UserContext *uctxt);

void TrapClock(UserContext *uctxt);

void CopyUserProcess (pte_t* parentPtb, pte_t* childPtb);

int switchproc(lstnode* switchOut, lstnode* switchIn);
// int traverseParent(lstnode *procnode);              //Traverse through its parent
// int traverseChildren(lstnode *procnode);            //Traverse through its 
// int apppendProcess(lstnode *procnode, pcb_t *src);     //Add a process in PCB list
// int removeProcessBypid(pcb_t *des, int pid);   //Remove a process from PCB by PID
// int findProcessBypid(pcb_t *des, int pid);      //Find a process by PID

int enreadyqueue(lstnode *procnode,dblist* queue);
lstnode* dereadyqueue(dblist* queue);
int enwaitingqueue(lstnode *procnode,dblist* queue);
lstnode* dewaitingqueue(lstnode* waitingnode,dblist* queue);

lstnode* TurnPCBToNode(pcb_t *pcb);
pcb_t* TurnNodeToPCB(lstnode *node);

void terminateProcess(lstnode *procnode);