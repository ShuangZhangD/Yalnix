#include "yalnix.h"
#include "hardware.h"
#include "lock.h"

dblist *lockqueue;
extern lstnode *currProc;
extern dblist* readyqueue;

int kernellockinit(UserContext *uctxt){

	int id = getMutexId();
	int *lock_idp =(int *) uctxt->regs[0];

    int rc = InputSanityCheck(lock_idp);
    if (rc){
        TracePrintf(1, "Error!The lock_idp address:%d in kernellockinit is not valid!\n", lock_idp);
    }

	lstnode *lockNode = nodeinit(id);
	if (NULL == lockNode){
		return ERROR;
	}

	lock_t *lock = (lock_t*)MallocCheck(sizeof(lock_t));
	if (NULL == lock){
		TracePrintf(1, "Malloc Failed! Get a NULL lock in kernellockinit!\n");  
		return ERROR;
	}

	lock->lock_id = id;
	lock->ownerid = -1;

	lock->waitlist = listinit();

	lockNode->content = (void *) lock;
	insert_tail(lockNode, lockqueue);
	*lock_idp = lock->lock_id;

    return SUCCESS;
}

int kernelaquire(UserContext *uctxt){
    
    //try to acquire the lock with the lock_id
    //if the lock is available, get the lock
    //if the lock is owned by others, get on the waitlist
    //if the lock is owned by itself, return message

	int lockId = uctxt->regs[0];
    return AcquireLock(lockId);
}

int kernelrelease(UserContext *uctxt){
	
	//try to release the lock with the lock_id
	//if the lock is owned by itslef, release the lock
	//if release is successful, the lock will be avalable
	//check the waitlist to see if someone is trying to get the lock
	//if the lock is owned by others, return error

	int lockId = uctxt->regs[0];
	if (isemptylist(lockqueue)) return ERROR;


    return ReleaseLock(lockId);
}

int AcquireLock(int lock_id){
	lstnode *locknode = search_node(lock_id, lockqueue);
	if (locknode == NULL){
		return ERROR;
	}

	lock_t *lock = (lock_t*) locknode->content;

	while (lock->ownerid != -1 && lock->ownerid != currProc->id){
		enwaitlockqueue(currProc,lock->waitlist);
		switchnext();
	}

	TracePrintf(1, "Owner Id:%d, currProc:%d\n",lock->ownerid, currProc->id);
	if (lock->ownerid == -1){
		lock->ownerid = currProc->id;
		return SUCCESS;
	} 

	return ERROR;
}


int ReleaseLock(int lock_id){

	lstnode* locknode = search_node(lock_id,lockqueue);
	if (locknode == NULL){
		return ERROR;
	}

	lock_t *lock = (lock_t *) locknode->content;
	if (lock->ownerid != currProc->id){
		return ERROR;
	}

	lock->ownerid = -1;
	if (!isemptylist(lock->waitlist)){
		lstnode* node = dewaitlockqueue(lock->waitlist);
		enreadyqueue(node,readyqueue);
	}

	return SUCCESS;
}


