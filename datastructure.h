typedef struct node{
 	int id;
 	// struct node *pre;
	// struct node *next;
} dblist;

dblist* listinit();
int isemptylist(dblist* list);
void insert_node(int i, int p, dblist* list);
void delete_node(int p,dblist* list);
void traverselist(dblist* list);

//use queue to implement all the waiting queues
typedef struct queue{

    int id;
    // dblist* head;
    // dblist* tail;

} dlqueue;


dlqueue* queueinit(dblist* f);
void enqueue(dlqueue* queue,int frameid);
void dequeue(dlqueue *queue);
int IsEmpty(dlqueue *queue);