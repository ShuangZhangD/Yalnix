#include "yalnix.h"
#include "hardware.h"
#include "pipe.h"

dblist* pipequeue;
extern int g_mutex_id;
extern lstnode* currProc;
extern dblist* readyqueue;

int kernelpipeinit(UserContext *uctxt){
	
	int *pipe_idp =(int *) uctxt->regs[0];

    int rc = InputSanityCheck(pipe_idp);
	if (rc){
        TracePrintf(1, "Error!The pipe_idp address:%d in kernelpipeinit is not valid!\n", pipe_idp);
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
		TracePrintf(1, "Malloc Failed! Get a NULL pipe in kernelpipeinit!\n");  
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

int kernelpiperead(UserContext *uctxt){
	int pipe_id = uctxt->regs[0];
	void* buf =(void*) uctxt->regs[1];
	int len = uctxt->regs[2];

    int rc = InputSanityCheck((int *)buf);
	if (rc){
        TracePrintf(1, "Error!The buf address:%p in kernelpiperead is not valid!\n", buf);
        return ERROR;
    }

	//if len is valid, use pipe_id to get the pipe to read from
	if(len < 0 || len > PIPE_BUFFER_LEN){
		TracePrintf(1, "Error! Invalid length in kernelpiperead!\n");
		return ERROR;
	}

	lstnode *pipenode = search_node(pipe_id, pipequeue);	
	if(pipenode == NULL){
		TracePrintf(1, "Error! cannot find pipe_id in kernelpiperead!\n");
		return ERROR;
	}

	pipe_t *pipe = (pipe_t *) pipenode->content;

	//if buffer < len, block the caller until there are enough bytes available

	while (len > pipe->contentlen) {
		enreaderwaitingqueue(currProc, pipe->readers);
		switchnext();			
	}
	
	memcpy(buf, &pipe->buffer, len);

	int leftbuflen = pipe->contentlen - len;
	if (len == PIPE_BUFFER_LEN){
		bzero(pipe->buffer, PIPE_BUFFER_LEN);
	} else {
		// TracePrintf(1, "Touch Address: %p\n", pipe->buffer);
		memcpy(&pipe->buffer, &pipe->buffer[len], leftbuflen);
		// TracePrintf(1, "After memcpy\n", pipe->buffer);
		memset(&pipe->buffer[leftbuflen], 0, PIPE_BUFFER_LEN-leftbuflen);
		pipe->contentlen = leftbuflen;
	}
	TracePrintf(3, "The content of buffer in kernelpiperead = %s\n", pipe->buffer);
	//if success, return the number of bytes read
	return len;
}

int kernelpipewrite(UserContext *uctxt){
	int pipe_id = uctxt->regs[0];
	unsigned int *buf = (unsigned int *) uctxt->regs[1];
	int len = uctxt->regs[2];

    int rc = InputSanityCheck((int *)buf);
	if (rc){
        TracePrintf(1, "Error!The buf address:%p in kernelpipewrite is not valid!\n", buf);
        return ERROR;
    }

	//if len is valid, use pipe_id to get the pipe to write to
	if(len < 0){
		TracePrintf(1, "Error! Invalid length in kernelpipewrite!\n");
		return ERROR;
	}	

	lstnode *pipenode = search_node(pipe_id, pipequeue);
	if(pipenode == NULL){
		TracePrintf(1, "Error! cannot find pipe_id in kernelpipewrite!\n");
		return ERROR;
	}

	pipe_t *pipe = (pipe_t *) pipenode->content;
	if (pipe->contentlen == PIPE_BUFFER_LEN){
		TracePrintf(1, "Error! PIPE is full!\n");
		return ERROR;
	}
	
	//read the buffer, and write the content to the pipe

	int bufferLeft = PIPE_BUFFER_LEN - pipe->contentlen;
	if (len > bufferLeft){
		TracePrintf(1, "Error! PIPE buffer only has %d bytes left, can't write %d bytes!!\n", bufferLeft, len);
		return ERROR;
	} else {
		int cpStInx = pipe->contentlen;
		memcpy(&pipe->buffer[cpStInx],buf,len);	
		pipe->contentlen+=len;

		if(!isemptylist(pipe->readers)){ 
			lstnode *node = dereaderwaitingqueue(pipe->readers);
			enreadyqueue(node, readyqueue);
		}
	}
	TracePrintf(3, "The content of buffer in kernelpipewrite= %s\n", pipe->buffer);
	//if success, return the number of bytes written
	return len;
}
