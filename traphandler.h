#include "selfdefinedstructure.h"
#include "memorymanage.h"

trapvector_t *intrptTb;

void TrapKernel(UserContext *uctxt);

void TrapClock(UserContext *uctxt);

void TrapIllegal(UserContext *uctxt);

void InitInterruptTable();

//Capture TRAP_TTY_RECEIVE
void TrapTtyReceive(UserContext *uctxt);

//Capture TRAP_TTY_TRANSMIT
void TrapTtyTransmit(UserContext *uctxt);

//Capture TRAP_DISK
void TrapDisk(UserContext *uctxt);
