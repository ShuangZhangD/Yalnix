#include <stdio.h>

void main(int argc, char const *argv[])
{
	/* code */
	int rc;
	int code = Fork();
	if (0 == code){
		TracePrintf(1,"I am child, code = %d\n", code);
		int pid = GetPid();
		TracePrintf(1,"Child Pid = %d\n", pid);
    	char *arglist[1];
    	arglist[0] = "testexec";
		Exec(arglist[0],arglist);
	} else {
		int *status_ptr;
		TracePrintf(1,"Before Waiting\n");
		rc = Wait(status_ptr);
		TracePrintf(1,"I am Parent, child pid = %d, exit child status:%d\n",code, status_ptr);
		int pid = GetPid();
		TracePrintf(1,"Patent Pid = %d\n", pid);

		Delay(10);
	}
	if (!code){
		Exit(-1);
	}

	int *status;
	Wait(&status);

	return;
}