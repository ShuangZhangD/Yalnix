int main(int argc, char const *argv[]){
	int *cvar_idp, *lock_idp, rc, condition = 0, pid;

	rc = LockInit(&lock_idp);
	if (rc){
		TracePrintf(1, "Lock Init Failed\n",GetPid());
		Exit(1);
	}

	rc = CvarInit(&cvar_idp);
	if (rc){
		TracePrintf(1, "Cvar Init Failed\n",GetPid());
		Exit(1);
	}

	pid = Fork();
	if (pid){
		pid = Fork();
	}

	if (GetPid() == 2){
		condition = 1;
	}

	//Protected By lock
	TracePrintf(1, "I am : %d, I am going to get the lock.\n",GetPid());
	rc = Acquire(lock_idp);
	if (rc) {
		TracePrintf(1, "I am : %d, I get lock failed.\n",GetPid());
		Exit(1);
	}
	TracePrintf(1, "I am : %d, I got the lock.\n",GetPid());


    while (!condition) {       
    	TracePrintf(1, "The condition is false, PID:%d wait (and release lock)\n",GetPid());       
    	CvarWait(cvar_idp,lock_idp);           
    	if (GetPid() == 3 || GetPid() == 4){
    		condition = 1;
    	} 
    	TracePrintf(1, "PID:%d: I'm back, with the lock.  I'll test again\n",GetPid());     
    }
	

	TracePrintf(1, "I am : %d, I print something.\n",GetPid());

	condition = 1;
	rc = Release(lock_idp);
	if (rc) {
		TracePrintf(1, "I am : %d, I release lock failed.\n",GetPid());
		Exit(1);
	}
	TracePrintf(1, "I am : %d, I release the lock, other people could come and get the lock.\n",GetPid());
	// Release Lock
	Delay(10);

	//Protected By lock
	TracePrintf(1, "I am : %d, I am going to get the second lock.\n",GetPid());
	rc = Acquire(lock_idp);
	if (rc) {
		TracePrintf(1, "I am : %d, I get lock failed.\n",GetPid());
		Exit(1);
	}
	TracePrintf(1, "I am : %d, I got the second lock.\n",GetPid());
	
	TracePrintf(1, "I am : %d, I print something, and make the condition true\n",GetPid());   
	condition = 1;     
	
	rc = CvarBroadcast(cvar_idp);       
	if (rc){
		TracePrintf(1, "I am : %d, CvarSignal Failed.\n",GetPid());
		Exit(1);		
	}

	TracePrintf(1, "I am : %d, I am about to release second lock.\n",GetPid());

	rc = Release(lock_idp);
	if (rc) {
		TracePrintf(1, "I am : %d, I release second lock failed.\n",GetPid());
		Exit(1);
	}
	TracePrintf(1, "I am : %d, I release the second lock, other people could come and get the lock.\n",GetPid());

	TracePrintf(1, "PID:%d, GetPid:%d\n", pid, GetPid());
	if (!pid){
		Exit(0);
	}

    int status;
    Wait(&status); 
    Wait(&status); 

	rc = Reclaim(lock_idp);
	if (rc) {
		TracePrintf(1, "I am : %d, I reclaim lock failed.\n",GetPid());
		Exit(1);
	}
	TracePrintf(1, "I am : %d, I reclaim lock success.\n",GetPid());
    
	rc = Reclaim(cvar_idp);
	if (rc) {
		TracePrintf(1, "I am : %d, I reclaim cvar failed.\n",GetPid());
		Exit(1);
	}
	TracePrintf(1, "I am : %d, I reclaim cvar success.\n",GetPid());

	return 0;


}