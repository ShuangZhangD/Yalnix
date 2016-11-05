#include "selfdefinedstructure.h"
#include "memorymanage.h"

trapvector_t *intrptTb;

void TrapKernel(UserContext *uctxt);

void TrapIllegal(UserContext *uctxt);

void InitInterruptTable();

//Capture TRAP_DISK
void TrapDisk(UserContext *uctxt);
