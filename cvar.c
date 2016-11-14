#include "yalnix.h"
#include "hardware.h"
#include "cvar.h"

dblist *cvarqueue;
extern int g_mutex_id;
extern lstnode* currProc;
extern dblist *lockqueue;


int kernelcvarinit(UserContext *uctxt){

	//Create a new condition variable
	int *cvar_idp =(int *) uctxt->regs[0];
	//Create a new pipe
	lstnode* cvarnode = nodeinit(getMutexId());

	if(cvarnode == NULL)
	{
		return ERROR;
	}

	cvar_t* cvar = (cvar_t*) malloc(sizeof(cvar_t));

	if(cvar == NULL)
	{
		return ERROR;
	}

	cvar->cvar_id = g_mutex_id;
	cvar->owner = NULL;
	cvar->cvarwaiting = NULL;
	cvarnode->content = cvar;
	insert_tail(cvarnode, cvarqueue);
	*cvar_idp = cvar->cvar_id;
	return 0;
	//save its identifier at *cvar_idp

}

int kernelcvarsignal(UserContext *uctxt){

	//if the cvar with cvar_id is not initialized, return ERROR
	int cvar_id = uctxt->regs[0];
	if(search_node(cvar_id, cvarqueue) == NULL)
	{
		return ERROR;
	}
	cvar_t* cvar = search_node(cvar_id, cvarqueue)->content;

	//while the waitlist of threads waiting for the cvar is not empty
	if (!isemptylist(cvar->cvarwaiting))
	{
		lstnode* node = decvarqueue(cvarqueue);
		enreadyqueue(node, readyqueue);
	}
	//signal one thread waiting for the cvar

    return ERROR;
}

int kernelcvarbroadcast(UserContext *uctxt){

	//if the cvar with cvar_id is not initialized, return ERROR
	int cvar_id = uctxt->regs[0];	
	
	if(search_node(cvar_id, cvarqueue) == NULL)
	{
		return ERROR;
	}
	cvar_t* cvar = search_node(cvar_id, cvarqueue)->content;

	//while the waitlist of threads waiting for the cvar is not empty
	while (!isemptylist(cvar->cvarwaiting))
	{
		lstnode* node = decvarqueue(cvarqueue);
		enreadyqueue(node, readyqueue);
	}
	//broadcast all threads waiting for the cvar

    return ERROR;
}

int kernelcvarwait(UserContext *uctxt){

	//if the cvar with cvar_id is not initialized, return ERROR
	int cvar_id = uctxt->regs[0];
	int lock_id = uctxt->regs[1];

	if(search_node(cvar_id, cvarqueue) == NULL)
	{
		return ERROR;
	}
	cvar_t* cvar = search_node(cvar_id, cvarqueue)->content;

	if(search_node(lock_id, lockqueue) == NULL)
	{
		return ERROR;
	}

	//release the lock identified by lock_id
	kernelrelease();
	//put the current thread on the waitlist, waiting on the cvar with cvar_id
	if (cvar->cvarwaiting == NULL)
	{
		cvar->cvarwaiting = listinit();
	}
	encvarqueue(currProc, cvar->cvarwaiting);
	switchproc();
	kernelaquire();

	//re-acquire the lock if waked up by signal or broadcast

    
    return ERROR;
}
