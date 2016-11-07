int main(int argc, char const *argv[])
{
	int rc;
	
	//Basic Case
	// rc = TtyPrintf(1, "Hello Yalnix! %d\n", 1);
	// if (rc) {
	// 	TracePrintf(1, "testio error\n");
	// }

	int len = 20;
	char buf1[4][20];
	int i;
	int *status;

	for (i = 0; i < 4; ++i) {
			
		int rc = Fork();
		if (rc == 0){
			TracePrintf(1, "I am a Child, pid = %d\n", GetPid());
			TtyRead(1, (void *)buf1[i], len);
			// TtyPrintf(1, "Hello Yalnix!  %d\n", GetPid());
			TracePrintf(1, "Buffer Content : %s\n", buf1[i]);
			exit(1);
		} else {
			TracePrintf(1, "I am a Parent, do nothing\n");
		}
		if (GetPid() == 2 && i == 3){
			while(1) {
				Pause();
				TracePrintf(1, "Great Parent Just wait here\n");
			}
		}		
	}

	// return 0;
}