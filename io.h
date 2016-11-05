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