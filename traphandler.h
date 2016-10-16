#define TRAP_SIZE 7
typedef void (*trapvector_t) (UserContext*);

trapvector_t *intrptTb;
