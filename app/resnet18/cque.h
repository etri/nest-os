#ifndef CQUE_H
#define CQUE_H

#include <pthread.h>

typedef struct cnode
{
        struct cnode *prev;
        struct cnode *next;
	int *value;
	int seq;
	double arr_time; // arrival time
	double arr_diff; // difference time between prev_arr_time and arr_time
} Cnode;

typedef struct cque
{
        Cnode *head;
        Cnode *tail;
        int count; /* count can be deleted. But it is important for debugging */
	pthread_mutex_t lock;
	double avg_arr_time;
	double prev_arr_time;
	int total_count;

        // method
        int (*push)(struct cque *q, Cnode *node);
        int (*push_sort)(struct cque *q, Cnode *node);
        Cnode *(*pop)(struct cque *q);
        Cnode *(*pop_sort)(struct cque *q, int prev_seq);
        int (*delete)(struct cque *q, Cnode *node);
        int (*is_empty)(struct cque *q);
} Cque;

Cque *cque_create(void);
Cnode *cnode_create(void);
void cque_free(Cque *q);
void cnode_free(Cnode *cn);

#if 0
void print_cque(Cque *q);
void print_node(Cnode *node);
#endif
#endif

