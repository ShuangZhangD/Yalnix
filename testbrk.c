int main(int argc, char const *argv[])
{
	int rc;
	rc = brk((void *)27000);  //Higher than brk
	if (rc){
		TracePrintf(1,"BRK FAILED\n");
	}
	rc = brk(0x4100);  //Lower than brk
	if (rc){
		TracePrintf(1,"BRK FAILED\n");
	}
	rc = brk(0xfeeee);  //Invalid Higher brk
	if (rc){
		TracePrintf(1,"BRK FAILED\n");
	}
	rc = brk(0x1000);  //Invalid lower brk
	if (rc){
		TracePrintf(1,"BRK FAILED\n");
	}

	return 0;
}