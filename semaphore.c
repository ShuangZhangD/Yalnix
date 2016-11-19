#include "semaphore.h"

dblist *semqueue;
extern lstnode *currProc;
extern dblist* readyqueue;

int KernelSemInit(UserContext *uctxt){
	int id = getMutexId();
	int *sem_idp =(int *) uctxt->regs[0];
	int semval = (int) uctxt->regs[1];

    int rc = InputSanityCheck(sem_idp);
    if (rc){
        TracePrintf(1, "Error!The sem_idp address:%d in KernelSemInit is not valid!\n", sem_idp);
        return ERROR;
    }

	lstnode *semNode = nodeinit(id);
	if (NULL == semNode){
		return ERROR;
	}

	sem_t *sem = (sem_t*)MallocCheck(sizeof(sem_t));
	if (NULL == sem){
		TracePrintf(1, "Malloc Failed! Get a NULL semaphore in KernelSemInit!\n");  
		return ERROR;
	}

	sem->sem_id = id;
	sem->ownerid = -1;
	sem->sem_val = semval;

	sem->semwaitlist = listinit();

	semNode->content = (void *) sem;
	
	insert_tail(semNode, semqueue);
	
	*sem_idp = sem->sem_id;

	return SUCCESS;
}

int KernelSemUp(UserContext *uctxt){
	int id = (int) uctxt->regs[0];
	lstnode* semNode = search_node(id, semqueue);

	if(semNode == NULL)
	{
		return ERROR;
	}
	sem_t* sem = (sem_t*)semNode->content;
	sem->sem_val++;

	if(!isemptylist(sem->semwaitlist))
	{
		lstnode* node = dewaitsemqueue(sem->semwaitlist);
		enreadyqueue(node, readyqueue);
	}
	return SUCCESS;
}

int KernelSemDown(UserContext *uctxt){
	int id = (int) uctxt->regs[0];
	lstnode* semNode = search_node(id, semqueue);

	if(semNode == NULL)
	{
		return ERROR;
	}
	sem_t* sem = (sem_t*)semNode->content;

	pcb_t* proc =(pcb_t*) currProc->content;
	if(sem->ownerid == -1)
	{
		sem->sem_id = proc->pid;
	}else if(sem->ownerid != proc->pid)
	{
		return ERROR;
	}
	
	
	while (sem->sem_val <= 0)
	{
		enwaitsemqueue(currProc, sem->semwaitlist);
		switchnext();
	}
	sem->sem_val--;
	
	return SUCCESS;
}