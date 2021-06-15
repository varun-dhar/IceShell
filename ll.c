#include "ll.h"

typedef struct{}

typedef struct {

#define LL(type,name) struct name_list{\
struct name_node{type data; struct name_node* next;}*head,*tail;}*name;
#define LL_NODE(list) typeof(list->head)
#define LL_NEW(name) name = malloc(sizeof(*name));\
name->head = name->tail = NULL
#define LL_PUSH(list,nData) if(!list->head){\
list->head = malloc(sizeof(*list->head)); list->tail = list->head;\
list->head->data = nData; list->head->next = NULL;}else{\
list->tail->next = malloc(sizeof(*list->tail)); list->tail = list->tail->next;\
list->tail->data = nData; list->tail->next = NULL;}
#define LL_HEAD(list) list->head
#define LL_TAIL(list) list->tail
#define LL_POP(list) if(list->head){typeof(list->head) tmp = list->head;\
list->head = list->head->next; list->tail=(!list->head)?NULL:list->tail;\
free(tmp);}
#define LL_DESTROY(list) while(list->head){\
LL_POP(list);}free(list);
