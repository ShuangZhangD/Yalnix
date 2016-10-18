#include "datastructure.h"
#include "listcontrol.h"
//use a doublylinkedlist to track free frame


dblist* listinit(dblist* list)
{
	int size = 0;
	list->head = NULL;
	list->tail = NULL;
}

lstnode* firstnode(dblist* list)
{
	return list->head;
}

int isemptylist(dblist* list)
{
	if (list->head->next = NULL)
		{
			return 0;
		}
	return 1;
}

void insert_tail(lstnode* node, dblist* list)
{
	node->pre = list->tail->pre;
	node->next = list->tail;
	list->tail->pre->next = node;
	list->tail->pre = node;
	list->size++;
}

void insert_head(lstnode* node, dblist* list)
{
	node->next = list->head->next ;
	node->pre = list->head;	
	list->head->next->pre = node;
	list->head->next = node;
	list->size++;
}

void remove_tail(dblist* list)
{
	list->tail->pre = list->tail->pre->pre;
	list->tail->pre = list->tail;
	list->size--;
}

void remove_head(dblist* list)
{
	list->head->next = list->head->next->next;
	list->head->next->pre = list->head;
	list->size--;
}

lstnode* search_node(lstnode* node,dblist* list)
{
	lstnode *h = list->head;
	if (isemptylist(list))
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
	lstnode *remove = search_node(node,list);
	remove->pre->next = remove->next;
	remove->next->pre = remove->pre;
	list->size--;
}

void traverselist(dblist* list)
{
	lstnode *traverse = list->head;
	if(isemptylist)
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







