#include "selfdefinedstructure.h"

int kernelttywrite(UserContext *uctxt);
int kernelttywrite(UserContext *uctxt);

typedef struct Terminal{
	dblist* reader;
	dblist* writer;
	char* transmitbuf;
	char* receivebuf;
} tty;

tty* Tty[NUM_TERMINALS];

//Capture TRAP_TTY_RECEIVE
void TrapTtyReceive(UserContext *uctxt);

//Capture TRAP_TTY_TRANSMIT
void TrapTtyTransmit(UserContext *uctxt);
