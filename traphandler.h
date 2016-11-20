#include "selfdefinedstructure.h"
#include "memorymanage.h"

trapvector_t *intrptTb;

//Capture TRAP_KERNEL
void TrapKernel(UserContext *uctxt);

//Capture TRAP_ILLEGAL
void TrapIllegal(UserContext *uctxt);

//Capture TRAP_MATH
void TrapMath(UserContext *uctxt);

//Capture TRAP_DISK
void TrapDisk(UserContext *uctxt);

// Initialize Interrupt Table
void InitInterruptTable();