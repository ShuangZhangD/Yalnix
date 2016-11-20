#include "yalnix.h"
#include "hardware.h"
#include "lock.h"

dblist *lockqueue;
extern lstnode *currProc;
extern dblist* readyqueue;

int KernelLockInit(UserContext *uctxt){
    TracePrintf(2, "Enter KernelLockInit\n");

	int id = getMutexId();
	int *lock_idp =(int *) uctxt->regs[0];

	//Check whether the address of lock_idp is valid!
    int rc = InputSanityCheck(lock_idp);
    if (rc){
        TracePrintf(1, "Error! The lock_idp address:%d in KernelLockInit is not valid!\n", lock_idp);
        return ERROR;
    }

	lstnode *lockNode = nodeinit(id);
	if (NULL == lockNode){
		return ERROR;
	}

	lock_t *lock = (lock_t*) MallocCheck(sizeof(lock_t));
	if (NULL == lock){
		TracePrintf(1, "Error! Malloc Failed! Get a NULL lock in KernelLockInit!\n");  
		return ERROR;
	}

	lock->lock_id = id;
	lock->ownerid = -1;

	lock->waitlist = listinit();

	lockNode->content = (void *) lock;
	insert_tail(lockNode, lockqueue);
	*lock_idp = lock->lock_id;

    TracePrintf(2, "Exit KernelLockInit\n");
    return SUCCESS;
}

int KernelLockAcquire(UserContext *uctxt){
    TracePrintf(2, "Enter KernelLockAcquire\n");
    //try to acquire the lock with the lock_id
	int lockId = uctxt->regs[0];
    return AcquireLock(lockId);
}

int KernelLockRelease(UserContext *uctxt){
	TracePrintf(2, "Enter KernelLockRelease\n");
	//try to release the lock with the lock_id
	int lockId = uctxt->regs[0];
	if (isemptylist(lockqueue)) {
		TracePrintf(1, "Error! There is no lock to release!\n");
		return ERROR;
	}
	TracePrintf(2, "Exit KernelLockRelease\n");
    return ReleaseLock(lockId);
}

int AcquireLock(int lock_id){
	lstnode *locknode = search_node(lock_id, lockqueue);
	if (locknode == NULL){
		return ERROR;
	}

	lock_t *lock = (lock_t*) locknode->content;

    //if the lock is owned by others, get on the waitlist
	while (lock->ownerid != -1 && lock->ownerid != currProc->id){
		enwaitlockqueue(currProc,lock->waitlist);
		switchnext();
	}

    //if the lock is available, get the lock
	TracePrintf(3, "Owner Id:%d, currProc:%d\n",lock->ownerid, currProc->id);
	if (lock->ownerid == -1){
		lock->ownerid = currProc->id;
		return SUCCESS;
	} 

	TracePrintf(1, "Error! Unexpected Behaviour in AcquireLock\n");
	return ERROR;
}


int ReleaseLock(int lock_id){

	//if release is successful, the lock will be avalable
	lstnode* locknode = search_node(lock_id,lockqueue);
	if (locknode == NULL){
		return ERROR;
	}

	//if the lock is owned by others, return error
	lock_t *lock = (lock_t *) locknode->content;
	if (lock->ownerid != currProc->id){
		return ERROR;
	}
	//if the lock is owned by itself, release the lock
	lock->ownerid = -1;
	
	//check the waitlist to see if someone is trying to get the lock
	if (!isemptylist(lock->waitlist)){
		lstnode* node = dewaitlockqueue(lock->waitlist);
		if (NULL == node){
			TracePrintf(1, "Error! dewaitlockqueue Failed in ReleaseLock!\n");
			return ERROR;
		}
		enreadyqueue(node,readyqueue);
	} 
	return SUCCESS;
}


