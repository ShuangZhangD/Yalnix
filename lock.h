#include "selfdefinedstructure.h"
#include "listcontrol.h"
#include "processmanage.h"

int KernelLockInit(UserContext *uctxt);

int KernelLockAcquire(UserContext *uctxt);

int KernelLockRelease(UserContext *uctxt);

int AcquireLock(int lock_id);

int ReleaseLock(int lock_id);