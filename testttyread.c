void testDifferentConsole();

int main(int argc, char const *argv[]){
	// testSameConsole();
	testDifferentConsole();
}


void testDifferentConsole(){
	int rc, i, len, maxi = 12;
	int buf[2048];
	int *status;

	for (i = 0; i < maxi; i++){
		int pid = Fork();
		if (0 == pid){
			TracePrintf(1 , "Child:%d  TTY_ID:%d\n", GetPid(), i%4);
			len = TtyRead(i%4, buf, 8);
			if (-1 == len){
				TracePrintf(1 , "TtyRead Failed!\n");
				Exit(-1);
			}
			TracePrintf(1, "You Shouldn't come here before reading from Console\n");
			TracePrintf(1, "Child:%d ReadLen = %d, buf = %s\n", GetPid(), len, buf);

			Exit(1);
		} else {
			Delay(10);
			TracePrintf(1 , "Parent:%d  TTY_ID:%d\n", GetPid(), i%4);
			len = TtyRead(i%4, buf, 3);
			if (rc == -1){
				TracePrintf(1, "TtyRead Failed!\n");
			}

			TracePrintf(1, "Parent:%d ReadLen = %d, buf = %s\n", GetPid(),len, buf);
		}
	}	

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

void testSameConsole(){
	int rc, i, len, maxi = 4;
	int buf[2048];
	int *status;

	for (i = 0; i < maxi; i++){
		int pid = Fork();
		if (0 == pid){
			len = TtyRead(1, buf, 8);
			if (-1 == len){
				TracePrintf(1 , "TtyRead Failed!\n");
				Exit(-1);
			}
			TracePrintf(1, "You Shouldn't come here before reading from Console\n");
			TracePrintf(1, "Child:%d ReadLen = %d, buf = %s\n", GetPid(), len, buf);

			Exit(1);
		} else {
			Delay(10);
			len = TtyRead(1, buf, 3);
			if (rc == -1){
				TracePrintf(1, "TtyRead Failed!\n");
			}

			TracePrintf(1, "Parent:%d ReadLen = %d, buf = %s\n", GetPid(),len, buf);
		}
	}	

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