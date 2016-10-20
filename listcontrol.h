#include "datastructure.h"

lstnode* nodeinit(int i);
dblist* listinit();
lstnode* firstnode(dblist* list);
int isemptylist(dblist* list);
void insert_tail(lstnode* node, dblist* list);
void insert_head(lstnode* node, dblist* list);
lstnode* remove_tail(dblist* list);
lstnode* remove_head(dblist* list);
lstnode* search_node(lstnode* node,dblist* list);
void remove_node(lstnode* node,dblist* list);
void traverselist(dblist* list);
