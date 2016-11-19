#include <stdio.h>

void main(int argc, char const *argv[])
{
	/* code */
	int rc, pid;
	int status;
	pid = Fork();
	if (0 == pid){
		TracePrintf(1,"I am child, code = %d\n", pid);
		TracePrintf(1,"Child Pid = %d\n", GetPid());
    	// char *args[1];
    	// args[0] = "testexec";
		// rc = Exec(args[0],args);
		// if (rc){
		// 	TracePrintf(1, "Exec Failed!\n");
		// }
		Exit(3);
	} else {
		TracePrintf(1,"I am Parent, child pid = %d\n",pid);
		TracePrintf(1,"Patent Pid = %d\n", GetPid());

		Delay(10);
	}
	if (!pid){
		TracePrintf(1, "Exit PID: %d\n", GetPid());	
		Exit(-2);
	}

	TracePrintf(1, "address: %p\n", &status);
	Wait(&status);
	TracePrintf(1, "Status: %d\n", status);

	return;
}