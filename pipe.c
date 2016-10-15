#include"yalnix.h"
#include"hardware.h"

int kernelpipeinit(int *pipe_idp){
	
	//Create a new pipe

	//save its identifier at *pipe_idp

return ERROR;
}

int kernelpiperead(int pipe_id, void *buf, int len){

	//if len is valid, use pipe_id to get the pipe to read from

	//if buffer < len, block the caller until there are enough bytes available

	//if success, return the number of bytes read

return ERROR;
}

int kernelpipewrite(int pipe_id, void *buf, int len){

	//if len is valid, use pipe_id to get the pipe to write to

	//read the buffer, and write the content to the pipe

	//if success, return the number of bytes written

return ERROR;
}
