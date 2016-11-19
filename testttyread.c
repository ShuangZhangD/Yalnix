int main(int argc, char const *argv[])
{
	int rc;
	int buf[2048];
	int *status;
	int len;
	int pid = Fork();
	if (0 == pid){
		len = TtyRead(1, buf, 512);
		if (-1 == len){
			TracePrintf(1, "TtyRead Failed!\n");
			Exit(-1);
		}
		TracePrintf(1, "You Shouldn't come here before reading from Console\n");
		TracePrintf(1, "ReadLen = %d, buf = %s\n", len, buf);

		Exit(1);
	} else {

		Delay(10);
		len = TtyRead(1, buf, 26);
		if (rc == -1){
			TracePrintf(1, "TtyRead Failed!\n");
		}

		TracePrintf(1, "ReadLen = %d, buf = %s\n", len, buf);
	}

	Wait(&status);
	return 0;
}