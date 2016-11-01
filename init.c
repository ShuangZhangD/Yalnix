int main(int argc, char const *argv[])
{
	int rc;
	while (1){
		TracePrintf(1, "Init looping...... \n");
		rc = Delay(100);
		TracePrintf(1, "rc = %d", rc);
	}
	return 0;
}