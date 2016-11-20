#include "yalnix.h"
#include "hardware.h"
#include "selfdefinedstructure.h"
#include "listcontrol.h"
#include "kernel.h"

//Global variables!
dblist* waitingqueue;
dblist* readyqueue;
dblist* blockqueue;

/*
	Syscalls & Traps
*/
int KernelDelay(UserContext *uctxt);

int KernelFork(UserContext *uctxt);

int KernelExec(UserContext *uctxt);

int KernelExit(UserContext *uctxt);

int KernelWait(UserContext *uctxt);

int KernelGetPid();

int KernelDelay(UserContext *uctxt);

void TrapClock(UserContext *uctxt);

/*
	Syscalls & Traps
*/

int CheckAvailableFrame(lstnode *cur_p);

void CopyUserProcess (pte_t* parentPtb, pte_t* childPtb);

int switchproc();

int switchnext();

int ProcessExit();

int enreadyqueue(lstnode *procnode,dblist* queue);
lstnode* dereadyqueue(dblist* queue);

int enwaitingqueue(lstnode *procnode,dblist* queue);
lstnode* dewaitingqueue(lstnode* waitingnode,dblist* queue);

int enblockqueue(lstnode* procnode,dblist* queue);
lstnode* deblockqueue(lstnode* waitingnode,dblist* queue);

int enreaderwaitingqueue(lstnode* procnode,dblist* queue);
lstnode* dereaderwaitingqueue(dblist* queue);
int enwriterwaitingqueue(lstnode* procnode,dblist* queue);
lstnode* dewriterwaitingqueue(dblist* queue);

int enwaitlockqueue(lstnode* procnode,dblist* queue);
lstnode* dewaitlockqueue(dblist* queue);

int encvarqueue(lstnode* procnode,dblist* queue);
lstnode* decvarqueue(dblist* queue);

int enwaitcvarqueue(lstnode* procnode,dblist* queue);
lstnode* dewaitcvarqueue(dblist* queue);

int enwaitsemqueue(lstnode* procnode,dblist* queue);
lstnode* dewaitsemqueue(dblist* queue);

int enbufferqueue(lstnode* procnode,dblist* queue);
lstnode* debufferqueue(dblist* queue);  

lstnode* TurnPCBToNode(pcb_t *pcb);
pcb_t* TurnNodeToPCB(lstnode *node);

void terminateProcess(lstnode *procnode);