#include "selfdefinedstructure.h"
#include "listcontrol.h"
#include "processmanage.h"


int KernelPipeInit(UserContext *uctxt);

/*
	1. Can't read buffer size > PIPE_BUFFER_LEN, will return an error
*/
int KernelPipeRead(UserContext *uctxt);

/*
	We have a non-blocking pipe, which:
	1. Allow only PIPE_BUFFER_LEN bytes of data existing into in the pipe
	2. Allow users to give a len > [available bytes left in the pipe], but input bytes are partially written,
	   only the size of [available bytes left in the pipe] could be writte and Yalnix will discard unwritten data. 
*/
int KernelPipeWrite(UserContext *uctxt);