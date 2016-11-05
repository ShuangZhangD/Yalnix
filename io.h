int kernelttywrite(UserContext *uctxt);
int kernelttywrite(UserContext *uctxt);

//Capture TRAP_TTY_RECEIVE
void TrapTtyReceive(UserContext *uctxt);

//Capture TRAP_TTY_TRANSMIT
void TrapTtyTransmit(UserContext *uctxt);