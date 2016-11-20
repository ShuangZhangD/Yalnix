#include "yalnix.h"
#include "hardware.h"
#include "pipe.h"

dblist* pipequeue;
extern int g_mutex_id;
extern lstnode* currProc;
extern dblist* readyqueue;

int KernelPipeInit(UserContext *uctxt){
	
	int *pipe_idp =(int *) uctxt->regs[0];

    int rc = InputSanityCheck(pipe_idp);
	if (rc){
        TracePrintf(1, "Error!The pipe_idp address:%d in KernelPipeInit is not valid!\n", pipe_idp);
        return ERROR;
    }

	//Create a new pipe
	lstnode* pipenode = nodeinit(getMutexId());
	if(pipenode == NULL){
		TracePrintf(1, "Error! pipenode is NULL!\n");  
		return ERROR;
	}

	pipe_t *pipe = (pipe_t *) MallocCheck(sizeof(pipe_t));
	if(pipe == NULL) {
		TracePrintf(1, "Malloc Failed! Get a NULL pipe in KernelPipeInit!\n");  
		return ERROR;
	}

	pipe->id = g_mutex_id;
	pipe->contentlen = 0;
	pipe->readers = listinit();
	pipenode->content = (void *) pipe;
	insert_tail(pipenode, pipequeue);

	//save its identifier at *pipe_idp
	*pipe_idp = pipe->id;
	
	return SUCCESS;
}

int KernelPipeRead(UserContext *uctxt){
	TracePrintf(2, "Enter KernelPipeRead\n");

	int pipe_id = uctxt->regs[0];
	void* buf =(void*) uctxt->regs[1];
	int len = uctxt->regs[2];

    int rc = InputSanityCheck((int *)buf);
	if (rc){
        TracePrintf(1, "Error!The buf address:%p in KernelPipeRead is not valid!\n", buf);
        return ERROR;
    }

	//if len is valid, use pipe_id to get the pipe to read from
	if(len < 0 || len > PIPE_BUFFER_LEN){
		TracePrintf(1, "Error! Invalid length in KernelPipeRead!\n");
		return ERROR;
	}

	lstnode *pipenode = search_node(pipe_id, pipequeue);	
	if(pipenode == NULL){
		TracePrintf(1, "Error! cannot find pipe_id in KernelPipeRead!\n");
		return ERROR;
	}

	pipe_t *pipe = (pipe_t *) pipenode->content;

	//if buffer < len, block the caller until there are enough bytes available
	while (len > pipe->contentlen) {
		enreaderwaitingqueue(currProc, pipe->readers);
		switchnext();			
	}
	
	memcpy(buf, &pipe->buffer, len);
	/*
		if user read all the pipe at once => memcpy and bzero all the buffers
		else pull the unread data to the front end of the pipe
	*/
	int leftbuflen = pipe->contentlen - len;
	if (len == PIPE_BUFFER_LEN){
		bzero(pipe->buffer, PIPE_BUFFER_LEN);
		pipe->contentlen = 0;
	} else {
		// TracePrintf(1, "Touch Address: %p\n", pipe->buffer);
		memcpy(&pipe->buffer, &pipe->buffer[len], leftbuflen);
		// TracePrintf(1, "After memcpy\n", pipe->buffer);
		memset(&pipe->buffer[leftbuflen], 0, PIPE_BUFFER_LEN-leftbuflen);
		pipe->contentlen = leftbuflen;
	}
	TracePrintf(3, "The content of buffer in KernelPipeRead = %s\n", pipe->buffer);

	TracePrintf(2, "Exit KernelPipeRead\n");	
	//if success, return the number of bytes read
	return len;
}

int KernelPipeWrite(UserContext *uctxt){
 	TracePrintf(2, "Enter KernelPipeWrite\n");

	int pipe_id = uctxt->regs[0];
	unsigned int *buf = (unsigned int *) uctxt->regs[1];
	int len = uctxt->regs[2];

    int rc = InputSanityCheck((int *)buf);
	if (rc){
        TracePrintf(1, "Error!The buf address:%p in KernelPipeWrite is not valid!\n", buf);
        return ERROR;
    }

	//if len is valid, use pipe_id to get the pipe to write to
	if(len < 0){
		TracePrintf(1, "Error! Invalid length in KernelPipeWrite!\n");
		return ERROR;
	}	

	lstnode *pipenode = search_node(pipe_id, pipequeue);
	if(pipenode == NULL){
		TracePrintf(1, "Error! cannot find pipe_id in KernelPipeWrite!\n");
		return ERROR;
	}

	//Cannot write into pipe when pipe is full
	pipe_t *pipe = (pipe_t *) pipenode->content;
	if (pipe->contentlen == PIPE_BUFFER_LEN){
		TracePrintf(1, "Error! PIPE is full!\n");
		return ERROR;
	}

	//read the buffer, and write the content to the pipe
	int bufferLeft = PIPE_BUFFER_LEN - pipe->contentlen;

	int writelen = (len > bufferLeft)? bufferLeft:len;
	int cpStInx = pipe->contentlen;
	memcpy(&pipe->buffer[cpStInx],buf,writelen);	
	pipe->contentlen+=writelen;

	if(!isemptylist(pipe->readers)){ 
		lstnode *node = dereaderwaitingqueue(pipe->readers);
		enreadyqueue(node, readyqueue);
	}

	// if (len > bufferLeft){
	// 	int cpStInx = pipe->contentlen;
	// 	memcpy(&pipe->buffer[cpStInx],buf,bufferLeft);	
	// 	pipe->contentlen+=bufferLeft;
	// 	// TracePrintf(1, "Error! PIPE buffer only has %d bytes left, can't write %d bytes!!\n", bufferLeft, len);
	// 	// return ERROR;
	// } else {
	// 	int cpStInx = pipe->contentlen;
	// 	memcpy(&pipe->buffer[cpStInx],buf,len);	
	// 	pipe->contentlen+=len;

	// 	if(!isemptylist(pipe->readers)){ 
	// 		lstnode *node = dereaderwaitingqueue(pipe->readers);
	// 		enreadyqueue(node, readyqueue);
	// 	}
	// }
	TracePrintf(3, "The content of buffer in KernelPipeWrite= %s\n", pipe->buffer);

 	TracePrintf(2, "Exit KernelPipeWrite\n");

	//if success, return the number of bytes written
	return writelen;
}
