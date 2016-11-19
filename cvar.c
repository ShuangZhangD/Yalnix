#include "yalnix.h"
#include "hardware.h"
#include "cvar.h"
#include "lock.h"

dblist *cvarqueue;
extern lstnode* currProc;

int KernelCvarInit(UserContext *uctxt){

	//Create a new condition variable
	int *cvar_idp = (int *) uctxt->regs[0];

    int rc = InputSanityCheck(cvar_idp);
    if (rc){
        TracePrintf(1, "Error!The cvar_idp address:%d in KernelLockInit is not valid!\n", cvar_idp);
        return ERROR;
    }

	//Create a new cvar
	int cvar_id = getMutexId();
	lstnode* cvarnode = nodeinit(cvar_id);
	if(cvarnode == NULL) {
		return ERROR;
	}

	cvar_t* cvar = (cvar_t*) MallocCheck(sizeof(cvar_t));
	if(cvar == NULL){
		TracePrintf(1, "Malloc Failed! Get a NULL cvar in KernelCvarInit!\n");  
		return ERROR;
	}

	cvar->cvar_id = cvar_id;
	cvar->cvarwaiting = listinit();
	cvarnode->content = (void *) cvar;
	insert_tail(cvarnode, cvarqueue);

	//save its identifier at *cvar_idp
	*cvar_idp = cvar->cvar_id;
	
	return SUCCESS;


}

int KernelCvarSignal(UserContext *uctxt){

	//if the cvar with cvar_id is not initialized, return ERROR
	int cvar_id = uctxt->regs[0];
	lstnode *cvarnode = search_node(cvar_id, cvarqueue);
	if(cvarnode == NULL){
		return ERROR;
	}
	cvar_t* cvar = (cvar_t *) cvarnode->content;

	//while the waitlist of threads waiting for the cvar is not empty 
	//signal one thread waiting for the cvar
	if (!isemptylist(cvar->cvarwaiting)){
		lstnode* node = dewaitcvarqueue(cvar->cvarwaiting);
		enreadyqueue(node, readyqueue);
	} else {
		return ERROR;
	}
	
    return SUCCESS;
}

int KernelCvarBroadcast(UserContext *uctxt){

	//if the cvar with cvar_id is not initialized, return ERROR
	int cvar_id = uctxt->regs[0];	
	lstnode *cvarnode = search_node(cvar_id, cvarqueue);
	if(cvarnode == NULL){
		return ERROR;
	}
	cvar_t* cvar = (cvar_t *) cvarnode->content;

	//while the waitlist of threads waiting for the cvar is not empty
	while (!isemptylist(cvar->cvarwaiting))
	{
		lstnode* node = dewaitcvarqueue(cvar->cvarwaiting);
		enreadyqueue(node, readyqueue);
	}
	//broadcast all threads waiting for the cvar

    return SUCCESS;
}

int KernelCvarWait(UserContext *uctxt){
	TracePrintf(2, "Enter kernelcvarwait\n");
	//if the cvar with cvar_id is not initialized, return ERROR
	int rc;
	int cvar_id = uctxt->regs[0];
	int lock_id = uctxt->regs[1];

	lstnode *cvarnode = search_node(cvar_id, cvarqueue);
	if(cvarnode == NULL) {
		return ERROR;
	}

	cvar_t* cvar = (cvar_t *) cvarnode->content;

	//release the lock identified by lock_id
	rc = ReleaseLock(lock_id);
	if (rc){
		TracePrintf(1, "Cvar Wait calling ReleaseLock Failed!\n");
		return ERROR;
	}

	//put the current thread on the waitlist, waiting on the cvar with cvar_id
	enwaitcvarqueue(currProc, cvar->cvarwaiting);
	switchnext();

	//re-acquire the lock if waked up by signal or broadcast
	rc = AcquireLock(lock_id);
	if (rc){
		TracePrintf(1, "AcquireLock in KernelCvarWait Failed!\n");
	}

	TracePrintf(2, "Exit kernelcvarwait\n");
    return SUCCESS;
}
