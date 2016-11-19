#include "yalnix.h"
#include "hardware.h"
#include "selfdefinedstructure.h"
#include "listcontrol.h"
#include "kernel.h"

dblist* waitingqueue;
dblist* readyqueue;
dblist* blockqueue;

int kerneldelay(UserContext *uctxt);

int kernelfork(UserContext *uctxt);

int kernelexec(UserContext *uctxt);

int kernelexit(UserContext *uctxt);

int kernelwait(UserContext *uctxt);

int kernelgetpid();

int kerneldelay(UserContext *uctxt);

void TrapClock(UserContext *uctxt);

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

lstnode* TurnPCBToNode(pcb_t *pcb);
pcb_t* TurnNodeToPCB(lstnode *node);

void terminateProcess(lstnode *procnode);