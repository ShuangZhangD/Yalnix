#include "datastructure.h"

lstnode* nodeinit(int i);
dblist* listinit(dblist* list);
lstnode* firstnode(dblist* list);
int isemptylist(dblist* list);
void insert_tail(lstnode* node, dblist* list);
void insert_head(lstnode* node, dblist* list);
void remove_tail(dblist* list);
void remove_head(dblist* list);
lstnode* search_node(lstnode* node,dblist* list);
void remove_node(lstnode* node,dblist* list);
void traverselist(dblist* list);

