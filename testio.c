int main(int argc, char const *argv[])
{
	int rc;
	
	//Basic Case
	// rc = TtyPrintf(1, "Hello Yalnix! %d\n", 1);
	// if (rc) {
	// 	TracePrintf(1, "testio error\n");
	// }

	int len = 20;
	char* buf1;
	TtyRead(1, buf1, len);
	TracePrintf(1, "Buffer Content : %s\n", buf1);

	return 0;
}