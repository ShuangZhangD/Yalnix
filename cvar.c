#include"yalnix.h"
#include"hardware.h"

int kernelcvarinit(int *cvar_idp){

	//Create a new condition variable

	//save its identifier at *cvar_idp

    return ERROR;
}

int kernelcvarsignal(int cvar_id){

	//if the cvar with cvar_id is not initialized, return ERROR

	//while the waitlist of threads waiting for the cvar is not empty

	//signal one thread waiting for the cvar

    return ERROR;
}

int kenrnelcvarbroadcast(int cvar_id){

	//if the cvar with cvar_id is not initialized, return ERROR

	//while the waitlist of threads waiting for the cvar is not empty

	//broadcast all threads waiting for the cvar

    return ERROR;
}

int kernelcvarwait(int cvar_id, int lock_id){

	//if the cvar with cvar_id is not initialized, return ERROR

	//release the lock identified by lock_id

	//put the current thread on the waitlist, waiting on the cvar with cvar_id

	//re-acquire the lock if waked up by signal or broadcast

    
    return ERROR;
}
