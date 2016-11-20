#include <stdio.h>

void main(int argc, char const *argv[])
{
	/* code */
	int rc, i, pid ,maxi = 4;
	int pipe_idp;
	char buf[256];
	char test[] = "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88";

	rc = PipeInit(&pipe_idp);
	if (rc){
		TracePrintf(1, "Error! PipeInit Failed!\n");
		Exit(1);
	}
	for (i = 0; i < maxi; i++){
		pid = Fork();
		if (0 == pid){
			rc = PipeRead(pipe_idp, buf, 100);
			if (rc == -1){
				TracePrintf(1, "PipeRead Failed!\n");
				Exit(-1);
			}
			TracePrintf(1, "You Shouldn't come here before reading from Parent\n");
			TracePrintf(1, "ReadLen = %d, buf = %s, pid=%d\n", rc, buf, GetPid());

			rc = PipeRead(pipe_idp, buf, 200);
			if (rc == -1){
				TracePrintf(1, "PipeRead Failed!\n");
				Exit(-1);
			}
			TracePrintf(1, "ReadLen = %d, buf = %s, pid=%d\n", rc, buf, GetPid());

			Exit(1);
		} else {
			Delay(10);

			rc = PipeWrite(pipe_idp, test, 26);
			TracePrintf(1, "WriteLen = %d, pid=%d\n", rc, GetPid());
			if (rc == -1){
				TracePrintf(1, "PipeWrite Failed!\n");
				Exit(-1);
			}

			rc = PipeWrite(pipe_idp, test, 230);
			TracePrintf(1, "WriteLen = %d, pid=%d\n", rc, GetPid());
			if (rc == -1){
				TracePrintf(1, "PipeWrite Failed!\n");
				Exit(-1);
			}
		}
	}
	// int *status;
	// Wait(&status);

	if (GetPid()!=2){
		Exit(-1);
	}

	if (GetPid() == 2){
		while(1){
			Delay(10);
			TracePrintf(1, "looping........\n");
		}
	}

	return;
}