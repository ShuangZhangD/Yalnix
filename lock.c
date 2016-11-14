#include "yalnix.h"
#include "hardware.h"
#include "lock.h"

dblist *lockqueue;
extern lstnode *currProc;
extern dblist* readyqueue;

int kernellockinit(UserContext *uctxt){

	int id = getMutexId();
	int* lock_idp =(int *) uctxt->regs[0];
	lstnode *lockNode = nodeinit(id);
	lock_t *lock = (lock_t*)malloc(sizeof(lock_t));

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
	lstnode *node = search_node(lockId, lockqueue);
	if (node == NULL){
		return ERROR;
	}

	lock_t *lock = (lock_t*) node->content;

	if (lock->ownerid == -1){
		lock->ownerid = currProc->id;
		return SUCCESS;
	} else if (lock->ownerid != currProc->id){
		enwaitlockqueue(currProc,lock->waitlist);
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
	if (isemptylist(lockqueue)) return ERROR;

	lstnode* lockNode = search_node(lockId,lockqueue);

	lock_t *lock = (lock_t *) lockNode->content;
	if (lock->ownerid != currProc->id)
	{
		return ERROR;
	}
	lock->ownerid = -1;
	if (!isemptylist(lock->waitlist)){
		lstnode* node = dewaitlockqueue(lock->waitlist);
		enreadyqueue(node,readyqueue);
		switchnext();
	}




    return ERROR;
}

