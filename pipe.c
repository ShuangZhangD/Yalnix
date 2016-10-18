#include "yalnix.h"
#include "hardware.h"
#include "pipe.h"

int kernelpipeinit(UserContext *uctxt){
	
	//Create a new pipe

	//save its identifier at *pipe_idp

return ERROR;
}

int kernelpiperead(UserContext *uctxt){

	//if len is valid, use pipe_id to get the pipe to read from

	//if buffer < len, block the caller until there are enough bytes available

	//if success, return the number of bytes read

return ERROR;
}

int kernelpipewrite(UserContext *uctxt){

	//if len is valid, use pipe_id to get the pipe to write to

	//read the buffer, and write the content to the pipe

	//if success, return the number of bytes written

return ERROR;
}
