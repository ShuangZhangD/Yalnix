#include "datastructure.h"
#include "listcontrol.h"
//use a doublylinkedlist to track free frame

lstnode* nodeinit(int i)
{
	lstnode* node = (lstnode *)malloc(sizeof(lstnode));
	node->id = i;
	node->pre = NULL;
	node->next = NULL;
	return node;
}

dblist* listinit(dblist* list)
{
	
	lstnode* n = (lstnode *)malloc(sizeof(lstnode));
	list = (dblist* )malloc(sizeof(dblist));
	n->id = 0;
	n->pre = NULL;
	n->next = NULL;
	list->head = n;
	list->size = 0;
	list->tail = n;
	return list;
}

lstnode* firstnode(dblist* list)
{
	return list->head->next;
}

int isemptylist(dblist* list)
{
	if (!list || (list->size == 0 && list->head->next == NULL))
		{
			return 1;
		}
	return 0;
}

void insert_tail(lstnode* node, dblist* list)
{
	if (isemptylist(list))
	{
		node->next = list->tail;
		node->pre = list->head;
		list->head->next = node;
		list->tail->pre = node;
	}
	else{
	node->pre = list->tail->pre;
	node->next = list->tail;
	list->tail->pre->next = node;
	list->tail->pre = node;
	}
	list->size++;
}

void insert_head(lstnode* node, dblist* list)
{
	if(isemptylist(list))
	{
		node->next = list->tail;
		node->pre = list->head;
		list->head->next = node;
		list->tail->pre = node;
	}
	else{
	node->next = list->head->next ;
	node->pre = list->head;	
	list->head->next->pre = node;
	list->head->next = node;
	}
	list->size++;
}

void remove_tail(dblist* list)
{
	if(isemptylist(list))
	{
		printf("%s\n", "error");
	}
	else{
	list->tail->pre = list->tail->pre->pre;
	list->tail->pre = list->tail;
	list->size--;
	}
}

void remove_head(dblist* list)
{
	if(isemptylist(list))
	{
		printf("%s\n", "error");
	}
	else{
	list->head->next = list->head->next->next;
	list->head->next->pre = list->head;
	list->size--;
	}
}

lstnode* search_node(lstnode* node,dblist* list)
{
	lstnode *h = list->head;
	if (!isemptylist(list))
	{
		while (h != NULL && h->id != node->id)
		{
			h = h->next;
		}
	}
	return h;
}

void remove_node(lstnode* node, dblist* list)
{
	if(isemptylist(list))
	{
		printf("%s\n", "error");
	}
	else{
	lstnode *remove = search_node(node,list);
	remove->pre->next = remove->next;
	remove->next->pre = remove->pre;
	list->size--;
	}
}

void traverselist(dblist* list)
{
	lstnode *traverse = list->head;
	if(isemptylist(list))
	{
		printf("%s\n", "error");
	}
	else
		while(traverse != NULL)
			{
				traverse = traverse->next;
				printf("%s\n", traverse->id);
			}
}







