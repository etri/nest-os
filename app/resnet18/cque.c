#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "cque.h"

static int node_push(Cque *q, Cnode *node)
{
	pthread_mutex_lock(&q->lock);
        if (q==NULL || node == NULL)
        {
                printf("Cque or node is unavailable\n");
		pthread_mutex_unlock(&q->lock);
		exit(1);
        }

        if (q->head == NULL)
        {
                q->head = node;
                q->tail = node;
                node->prev = NULL;
                node->next = NULL;
        }
        else
        {
                node->prev = q->tail;
                node->next = NULL;
                q->tail->next = node;
                q->tail = node;
        }

        q->count++;

	pthread_mutex_unlock(&q->lock);
        return 0;
}

static int node_push_sort(Cque *q, Cnode *node)
{
	pthread_mutex_lock(&q->lock);
        if (!q)
	{
                printf("Cque is unavailable");
		pthread_mutex_unlock(&q->lock);
		exit(1);
	}

	if (q->head == NULL)
        {
                q->head = node;
                q->tail = node;
                node->prev = NULL;
                node->next = NULL;
        }
        else
        {
		Cnode *p;

       		for (p=q->head; p!=NULL; p=p->next)
        	{
			if (p->seq > node->seq)
			{
				break; // found
			}
        	}

		if (p) // found and insert it before p node
		{
                	node->prev = p->prev;
                	node->next = p;
			if (p->prev)
			{
				p->prev->next = node;
				p->prev = node;
			}

			// update q->head if p was the head of the q
			if (q->head == p)
			{
				q->head = node;
			}
		}
		else // not found, attach it to the last
		{
                	node->prev = q->tail;
                	node->next = NULL;

			// update q->tail
                	q->tail->next = node;
                	q->tail = node;
		}

#if 0 // for debug
	for (p=q->head; p!=NULL; p=p->next)
       	{
		printf("%d:", p->seq);
       	}
	printf("\n");
#endif
        }

        q->count++;

	//
	if (q->prev_arr_time < 0.000001)
	{
		node->arr_diff = 0.0;
	}
	else
	{
		node->arr_diff = node->arr_time - q->prev_arr_time; 
	}

	q->prev_arr_time = node->arr_time;

	q->total_count++;
	q->avg_arr_time = (q->avg_arr_time*(q->total_count-1) + node->arr_diff)/q->total_count;

	//printf("node->arr_diff = %f q->avg_arr_time = %f\n", node->arr_diff, q->avg_arr_time);

	pthread_mutex_unlock(&q->lock);

	return 0; // not found
}

static Cnode *node_pop(Cque *q)
{
	pthread_mutex_lock(&q->lock);
        if (!q)
        {
                printf("Cque is unavailable");
		pthread_mutex_unlock(&q->lock);
		exit(1);
        }

        Cnode *p;

        p = q->head;

        if (p) // found
        {
                q->head = p->next;

                if (p->next != NULL)
                {
                        p->next->prev = p->prev;
                }
                else
                {
                        q->tail = NULL;
                }

                q->count--;
        }

	pthread_mutex_unlock(&q->lock);
        return p;
}

static Cnode *node_pop_sort(Cque *q, int prev_seq)
{
	pthread_mutex_lock(&q->lock);
        if (!q)
        {
                printf("Cque is unavailable");
		pthread_mutex_unlock(&q->lock);
		exit(1);
        }

        Cnode *p;

        p = q->head;

        if (p) // found
        {
		if (prev_seq+1 == p->seq)
		{
                	q->head = p->next;

                	if (p->next != NULL)
                	{
                        	p->next->prev = p->prev;
                	}
                	else
                	{
                        	q->tail = NULL;
                	}

                	q->count--;
		}
		else
		{
			pthread_mutex_unlock(&q->lock);
			return NULL;
		}

        }

	pthread_mutex_unlock(&q->lock);
        return p;
}

static int node_delete(Cque *q, Cnode *node)
{
	pthread_mutex_lock(&q->lock);
        if (!q || !node)
	{
		pthread_mutex_unlock(&q->lock);
                return -1;
	}

        if (node->prev != NULL && node->next != NULL)
        {
                node->prev->next = node->next;
                node->next->prev = node->prev;
                node->next = NULL;
                node->prev = NULL;
        }
        else if (node->prev == NULL && node->next != NULL)
        {
                q->head = node->next;

                node->next->prev = NULL;
                node->next = NULL;
        }
        else if (node->prev != NULL && node->next == NULL)
        {
                q->tail = node->prev;

                node->prev->next = NULL;
                node->prev = NULL;
        }
        else /* p->prev == NULL; p->next == NULL */
        {
                q->head = q->tail = NULL;
        }

        q->count--;

	pthread_mutex_unlock(&q->lock);
        return 0;
}

static int queue_is_empty(Cque *q)
{
        return (!q->head);
}

Cnode *cnode_create(void)
{
	Cnode *node = malloc(sizeof(Cnode));
	if (!node)
	{
		printf("Error: node cannot be created\n");
		exit(1);
	}

	node->prev = node->next = NULL;

	node->value = NULL;
	node->seq = 0;
	node->arr_time = 0.0;
	node->arr_diff = 0.0;

	return node;
}

void cnode_free(Cnode *cn)
{
	//free(cn->value);
	free(cn);
}

Cque *cque_create(void)
{
	Cque *q = malloc(sizeof(Cque));
	if (!q)
	{
		printf("Error: q cannot be created\n");
		exit(1);
	}

        q->head = q->tail = NULL;
        q->count = 0;
	pthread_mutex_init(&q->lock, NULL);
	q->avg_arr_time = 0.0;
	q->prev_arr_time = 0.0;
	q->total_count = 0;

        q->push = node_push;
        q->push_sort = node_push_sort;
        q->pop = node_pop;
        q->pop_sort = node_pop_sort;
        q->delete = node_delete;
        q->is_empty = queue_is_empty;

	return q;
}

void cque_free(Cque *q)
{
        Cnode *p, *r;

        for (p=q->head; p!=NULL; p=r)
        {
                r=p->next; // r is temporal
                free(p);
        }
}

#if 0
/* This code is for debugging */
void print_queue(Cque *q)
{
        Cnode *p;

        for (p=q->head; p!=NULL; p = p->next)
        {
                sysLogOutMsg("value = %d\n", p->value);
        }
}

void print_node(Cnode *node)
{
        sysLogOutMsg("value = %d\n", node->value);
}
#endif

