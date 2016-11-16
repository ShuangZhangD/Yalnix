#include "selfdefinedstructure.h"
#include "listcontrol.h"
#include "processmanage.h"

int kernellockinit(UserContext *uctxt);

int kernelaquire(UserContext *uctxt);

int kernelrelease(UserContext *uctxt);

int AcquireLock(int lock_id);

int ReleaseLock(int lock_id);