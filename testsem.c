void main(int argc, char **argv) 
{
	int pid, sem_id, i, rc;
    
    rc = SemInit(&sem_id, 2);

    TtyPrintf(0, "%d: Successfully created sem with sem id %d\n", GetPid(), sem_id);

	for (i = 0; i < 4; ++i) {
			
		int rc = Fork();
		if (rc == 0){
			TracePrintf(1, "I am a Child, pid = %d\n", GetPid());
			// TtyRead(1, (void *)buf1[i], len);
			// TracePrintf(1, "Buffer Content : %s\n", buf1[i]);
			TtyPrintf(0, "%d: Attempting sem %d.\n", GetPid(), sem_id);
			rc = SemDown(sem_id);

			TtyPrintf(0, "%d: Sem Down %d.\n", GetPid(), sem_id);	
			// Delay(2);
			rc = SemUp(sem_id);

	    	TtyPrintf(0, "%d: Sem Up %d\n", GetPid(), sem_id);

	   	 	//TtyPrintf(0, "%d: Return value for previous attempt was %d\n", GetPid(), rc);
	    	Delay(2);
			exit(1);
		} else {
			TracePrintf(1, "I am a Parent, do nothing\n");
		}

    int status;
    TtyPrintf(0, "%d: Waiting children\n", GetPid());
    Wait(&status); 
    Wait(&status); 
    TtyPrintf(0, "%d: Waiting done\n", GetPid());

	rc = Reclaim(sem_id);

    TtyPrintf(0, "%d: Successfully reclaimed sem.\n", GetPid());
    TtyPrintf(0, "%d: End sem test.\n", GetPid());

 	if (GetPid() == 2 && i == 3){
			while(1) {
				Pause();
				TracePrintf(1, "Great Parent Just wait here\n");
			}
		}		
	}

}