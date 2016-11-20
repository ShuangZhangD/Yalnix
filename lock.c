#include "yalnix.h"
#include "hardware.h"
#include "lock.h"

dblist *lockqueue;
extern lstnode *currProc;
extern dblist* readyqueue;

int KernelLockInit(UserContext *uctxt){
	//Create a new lock
	int id = getMutexId();
	int *lock_idp =(int *) uctxt->regs[0];
	//Check whether the address is valid
    int rc = InputSanityCheck(lock_idp);
    if (rc){
        TracePrintf(1, "Error!The lock_idp address:%d in KernelLockInit is not valid!\n", lock_idp);
        return ERROR;
    }
    //initialize a lock node
	lstnode *lockNode = nodeinit(id);
	if (NULL == lockNode){
		return ERROR;
	}

	lock_t *lock = (lock_t*)MallocCheck(sizeof(lock_t));
	//check if malloc succeeds
	if (NULL == lock){
		TracePrintf(1, "Malloc Failed! Get a NULL lock in KernelLockInit!\n");  
		return ERROR;
	}
	//initialize the lock
	lock->lock_id = id;
	lock->ownerid = -1;

	lock->waitlist = listinit();

	// Put Cvar node into Cvar queue

	lockNode->content = (void *) lock;
	insert_tail(lockNode, lockqueue);
	//save its identifier at *lock_idp

	*lock_idp = lock->lock_id;

    return SUCCESS;
}

int KernelLockAcquire(UserContext *uctxt){
    
 	//acquire with id

	int lockId = uctxt->regs[0];
    return AcquireLock(lockId);
}

int KernelLockRelease(UserContext *uctxt){
	

	//release lock with id
	int lockId = uctxt->regs[0];
	if (isemptylist(lockqueue)) return ERROR;

    return ReleaseLock(lockId);
}

int AcquireLock(int lock_id){
    //get the lock with the lock_id	
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

	TracePrintf(3, "Owner Id:%d, currProc:%d\n",lock->ownerid, currProc->id);
    //if the lock is available, get the lock

	if (lock->ownerid == -1){
		lock->ownerid = currProc->id;
		return SUCCESS;
	} 
    //if the lock is owned by itself, return message

	return ERROR;
}
	

int ReleaseLock(int lock_id){
	//get the lock with the lock_id
	lstnode* locknode = search_node(lock_id,lockqueue);
	if (locknode == NULL){
		return ERROR;
	}

	lock_t *lock = (lock_t *) locknode->content;

	//if the lock is owned by others, return error

	if (lock->ownerid != currProc->id){
		return ERROR;
	}
	//if the lock is owned by itslef, release the lock	
	//if release is successful, the lock will be avalable

	lock->ownerid = -1;
	//check the waitlist to see if someone is trying to get the lock
	if (!isemptylist(lock->waitlist)){
		lstnode* node = dewaitlockqueue(lock->waitlist);
		enreadyqueue(node,readyqueue);
	}

	return SUCCESS;
}


