#include "selfdefinedstructure.h"
#include "memorymanage.h"

trapvector_t *intrptTb;

void TrapKernel(UserContext *uctxt);

void TrapIllegal(UserContext *uctxt);

void InitInterruptTable();

void TrapMath(UserContext *uctxt);


//Capture TRAP_DISK
void TrapDisk(UserContext *uctxt);
