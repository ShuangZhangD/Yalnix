#include "yalnix.h"
#include "hardware.h"
#include "pipe.h"
dblist* pipequeue;
extern int g_mutex_id;
extern lstnode* currProc;

int kernelpipeinit(UserContext *uctxt){
	
	int *pipe_idp =(int *) uctxt->regs[0];
	//Create a new pipe
	lstnode* pipenode = nodeinit(getMutexId());

	if(pipenode == NULL)
	{
		return ERROR;
	}

	Pipe* pipe = (Pipe*) malloc(sizeof(Pipe));

	if(pipe == NULL)
	{
		return ERROR;
	}

	pipe->id = g_mutex_id;
	pipe->contentlen = 0;
	pipe->readers = NULL;
	pipe->writers = NULL;
	pipenode->content = pipe;
	insert_tail(pipenode, pipequeue);
	*pipe_idp = pipe->id;
	return 0;
	//save its identifier at *pipe_idp

}

int kernelpiperead(UserContext *uctxt){
	int pipe_id = uctxt->regs[0];
	void* buf =(void*) uctxt->regs[1];
	int len = uctxt->regs[2];
	//if len is valid, use pipe_id to get the pipe to read from
	if(len < 0)
	{
		return ERROR;
	}

	if(search_node(pipe_id, pipequeue) == NULL)
	{
		return ERROR;
	}

	Pipe* pipe = search_node(pipe_id, pipequeue)->content;

	//if buffer < len, block the caller until there are enough bytes available


	if (len > pipe->contentlen)
	{
		if(pipe->readers == NULL)
		{
			pipe->readers = listinit();
		}
	enreaderwaitingqueue(currProc, pipe->readers);			
	
	}else{
		
	memcpy(buf, pipe->buffer, len);
	int leftbuflen = pipe->contentlen - len;
	int leftbuf[leftbuflen];
	memcpy(leftbuf, &pipe->buffer[len], leftbuflen);
	memcpy(pipe->buffer, leftbuf, leftbuflen);
	pipe->contentlen = leftbuflen;
	return len;
	}

	//if success, return the number of bytes read

}

int kernelpipewrite(UserContext *uctxt){
	int pipe_id = uctxt->regs[0];
	void* buf =(void*) uctxt->regs[1];
	int len = uctxt->regs[2];
	//if len is valid, use pipe_id to get the pipe to write to
	if(len < 0)
	{
		return ERROR;
	}

	if(search_node(pipe_id, pipequeue) == NULL)
	{
		return ERROR;
	}

	Pipe* pipe = search_node(pipe_id, pipequeue)->content;
	//read the buffer, and write the content to the pipe
	int totallen = pipe->contentlen + len;
	if(totallen > PIPE_BUFFER_LEN)
	{
		if(pipe->writers == NULL)
		{
			pipe->writers = listinit();
		}
	enwriterwaitingqueue(currProc, pipe->writers);
	}
	else{
		memcpy(&pipe->buffer[pipe->contentlen],buf,len);
		return len;
	}
	//if success, return the number of bytes written

return ERROR;
}
