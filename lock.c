#include "yalnix.h"
#include "hardware.h"
#include "lock.h"

dblist *lockqueue;

int kernellockinit(UserContext *uctxt){

	int id = getMutexId();
	uctxt->regs[0] = id;
	lstnode *lockNode = nodeinit(id);
	lock_t *lock = (lock_t*)malloc(sizeof(lock_t));

	lock->lock_id = id;
	lock->owner = NULL;

	lock->waitlist = listinit();

	lockNode->content = (void *) lock;
	enlockqueue(lockNode);

    return SUCCESS;
}

int kernelaquire(UserContext *uctxt){
    
    //try to acquire the lock with the lock_id

    //if the lock is available, get the lock

    //if the lock is owned by others, get on the waitlist

    //if the lock is owned by itself, return message





    return ERROR;
}

int kernelrelease(UserContext *uctxt){
	
	//try to release the lock with the lock_id

	//if the lock is owned by itslef, release the lock

	//if release is successful, the lock will be avalable
	
	//check the waitlist to see if someone is trying to get the lock


	//if the lock is owned by others, return error

    return ERROR;
}

