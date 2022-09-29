/*****************************************************************************
*
* Copyright Next-Generation System Software Research Group, All rights reserved.
* Future Computing Research Division, Artificial Intelligence Reserch Laboratory
* Electronics and Telecommunications Research Institute (ETRI)
*
* THESE DOCUMENTS CONTAIN CONFIDENTIAL INFORMATION AND KNOWLEDGE
* WHICH IS THE PROPERTY OF ETRI. NO PART OF THIS PUBLICATION IS
* TO BE USED FOR ANY OTHER PURPOSE, AND THESE ARE NOT TO BE
* REPRODUCED, COPIED, DISCLOSED, TRANSMITTED, STORED IN A RETRIEVAL
* SYSTEM OR TRANSLATED INTO ANY OTHER HUMAN OR COMPUTER LANGUAGE,
* IN ANY FORM, BY ANY MEANS, IN WHOLE OR IN PART, WITHOUT THE
* COMPLETE PRIOR WRITTEN PERMISSION OF ETRI.
*
* LICENSE file : README_LICENSE_ETRI located in the top directory
*
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "share_comm.h"
#include "server_npuos.h"

static void server_npuos_update_wcet(ServerNpuOS *nos, int nnid, int part_num_start, int part_num_delta, double new_wcet)
{
        Queue *q = nos->q[nnid];
        if (!q)
        {
                q = queue_create();
                nos->q[nnid] = q;
        }

        Qnode *node;

        node = q->search(q, part_num_start, part_num_delta);

        if (!node) // not found
        {
                node = node_create();
		pthread_mutex_init(&node->lock, NULL);

                node->part_num_start = part_num_start;
                node->part_num_delta = part_num_delta;
                node->wcet = new_wcet;

                q->push(q, node); // push the node
        }
        else // found the node and the update the wcet
        {
                if (node->wcet < new_wcet)
                {
			pthread_mutex_lock(&node->lock);
                        node->wcet = new_wcet;
			pthread_mutex_unlock(&node->lock);
                }
        }
}

static double server_npuos_get_wcet(ServerNpuOS *nos, int nnid, int part_num_start, int part_num_delta)
{
        Queue *q = nos->q[nnid];
        if (!q) // queue is empty
        {
                return 0;
        }

        Qnode *node;

        node = q->search(q, part_num_start, part_num_delta);
        if (!node) // not found
        {
                return 0;
        }
        else // found the node
        {
		pthread_mutex_lock(&node->lock);
		double wcet = node->wcet;
		pthread_mutex_unlock(&node->lock);

                return wcet;
        }
}

static void server_npuos_set_nnid(ServerNpuOS *nos, int nnid)
{
	int p, q;

	p = nnid/64;
	q = nnid%64;

	nos->nnid[p] |= (1 << q);
}

static void server_npuos_clear_nnid(ServerNpuOS *nos, int nnid)
{
	int p, q;

	p = nnid/64;
	q = nnid%64;

	nos->nnid[p] &= ~(1 << q);
}

static int server_npuos_is_nnid_set(ServerNpuOS *nos, int nnid)
{
	int p, q;

	p = nnid/64;
	q = nnid%64;

	return (nos->nnid[p] & (1 << q));
}

#if 0
static void print_all_nnid_set(ServerNpuOS *nos)
{
	for (int i=0; i<4; i++)
	{
		if (nos->nnid[i])
		{
			for (int j=0; j<64; j++)
			{
				if (nos->nnid[i] & (1 << j))
				{
					nnid = i*64 + j;
					printf("%d:", nnid);
				}
			}
		}
	}
}
#endif

ServerNpuOS *server_npuos_create(void)
{
        ServerNpuOS *npuos = malloc(sizeof(ServerNpuOS));
	if (!npuos)
	{
		printf("Error: npuos cannot be created\n");
		exit(1);
	}

        npuos->id = -1; // not determined yet

        npuos->state = OS_DORMANT;
        npuos->ntasks = 0;
	npuos->nos_fd = -1;

	for (int i=0; i<256; i++)
	{
		npuos->q[i] = NULL;
	}

	npuos->ewt = 0.0;
	//npuos->ntq = ntaskq_create();

	for (int i=0; i<4; i++)
	{
		npuos->nnid[i] = 0;
	}
        
	npuos->wsmem_size = 0;

	npuos->update_wcet = server_npuos_update_wcet;
	npuos->get_wcet = server_npuos_get_wcet;

	npuos->set_nnid = server_npuos_set_nnid;
	npuos->clear_nnid = server_npuos_clear_nnid;
	npuos->is_nnid_set = server_npuos_is_nnid_set;

        return npuos;
}

void server_npuos_free(ServerNpuOS *npuos)
{
        free(npuos);
}
