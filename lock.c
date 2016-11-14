#include "yalnix.h"
#include "hardware.h"
#include "lock.h"

dblist *lockqueue;
extern lstnode *currProc;

int kernellockinit(UserContext *uctxt){

	int id = getMutexId();
	uctxt->regs[0] = id;
	lstnode *lockNode = nodeinit(id);
	lock_t *lock = (lock_t*)malloc(sizeof(lock_t));

	lock->lock_id = id;
	lock->owner = NULL;

	lock->waitlist = listinit();

	lockNode->content = (void *) lock;
	enlockqueue(lockNode, lockqueue);

    return SUCCESS;
}

int kernelaquire(UserContext *uctxt){
    
    //try to acquire the lock with the lock_id
    //if the lock is available, get the lock
    //if the lock is owned by others, get on the waitlist
    //if the lock is owned by itself, return message

	int lockId = uctxt->regs[0];
	lstnode *node = search_node(lockqueue, lockId);
	if (node == NULL){
		return ERROR;
	}

	lock_t *lock = (lock_t*) node->content;

	if (lock->owner == NULL){
		node->owner = currProc;
		return SUCCESS;
	} else if (lock->owner != currProc){
		lstnode* node = nodeinit(currProc->id);
		enwaitlockqueue(node,lock->waitlist());
		switchnext();
		return SUCCESS;
	} else {
		TracePrintf(1, "Acquire Success");
		return SUCCESS;
	}

    return ERROR;
}

int kernelrelease(UserContext *uctxt){
	
	//try to release the lock with the lock_id
	//if the lock is owned by itslef, release the lock
	//if release is successful, the lock will be avalable
	//check the waitlist to see if someone is trying to get the lock
	//if the lock is owned by others, return error

	int lockId = uctxt->regs[0];
	if (isemptylist(lockqueue) || !currProc->hasLock || !currProc->lockId) return ERROR;

	lstnode* lockNode = search_node(currProc->lockId,lockqueue);

	lock_t *lock = (lock_t *) lockNode->content;
	if (!isemptylist(lock->waitlist)){
		
	}




    return ERROR;
}

