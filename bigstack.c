

int main()
{	
	// int a = 1024, b = 10;
	int a = 256, b = 32;
	int size = a*b;
	char big_buffer[size];
	// int foo;
	int i;

	// foo = 42;
	for (i = 0; i < size; i++) {
		TracePrintf(1,"a\n");
	 	big_buffer[i] = 'a';
	}
	// for (i = 0; i < size ;i+=b/2) 
	//   TracePrintf(0,"&big_buffer[%d] = %x; big_buffer[%d] = %c\n",
	// 	      i, &big_buffer[i], i, big_buffer[i]);

	return 0;
}
