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
#include <limits.h>
#include <float.h>
#include "scheduler.h"
#include "error.h"
#include "server_npuos.h"
#include "server_comm.h"
#include "share_comm.h"
#include "bit_handler.h"
#include "util_time.h"

#define SCHED_DBG	0

// mnt(min number of tasks)
static int scheduler_mnt(Scheduler *scheduler, int nnid, int part_num_start, int part_num_delta, unsigned int affinity_mask)
{
#if (SCHED_DBG==1)
	printf("scheduler_mnt :\n");
	double start_time = what_time_is_it_now();
#endif
	NnPos *nnpos = scheduler->nnpos[nnid];
	unsigned int bits;
	unsigned int mbits;
	int nos_r = -1;

	bits = nnpos->mask_nos_bit_match(nnpos, part_num_start, part_num_delta);

	mbits = bits & affinity_mask;

	int min_ntasks = INT_MAX;
	if (mbits)
	{
		for (int i=0; i<scheduler->npuos_num; i++)
	   	{
			if (mbits & (1<<i))
			{
				ServerNpuOS *os = scheduler->nos[i];
			
				if (min_ntasks > os->ntasks)
				{
					min_ntasks = os->ntasks;
					nos_r = i;
				}
			}
		}
	}

#if (SCHED_DBG==1)
	double time_duration = what_time_is_it_now() - start_time;
	printf("scheduling time = %f\n", time_duration);
	printf("Thus, scheduler selects NPU%d\n", nos_r);
#endif

	return nos_r;
}

// mwct(min worst-case completion time)
static int scheduler_mwct(Scheduler *scheduler, int nnid, int part_num_start, int part_num_delta, unsigned int affinity_mask)
{
#if (SCHED_DBG==1)
	printf("scheduler_mwct :\n");
	double start_time = what_time_is_it_now();
#endif
	//Scheduler *scheduler = server->scheduler;
	NnPos *nnpos = scheduler->nnpos[nnid];
	unsigned int bits;
	unsigned int mbits;
	int nos_r = -1;

	bits = nnpos->mask_nos_bit_match(nnpos, part_num_start, part_num_delta);

	mbits = bits & affinity_mask;

	double mintime = DBL_MAX;
	if (mbits)
	{
		for (int i=0; i<scheduler->npuos_num; i++)
	   	{
			if (mbits & (1<<i))
			{
				ServerNpuOS *nos = scheduler->nos[i];
				double wcet = nos->get_wcet(nos, nnid, part_num_start, part_num_delta); 
				if (mintime > nos->ewt+wcet)
				{
					mintime = nos->ewt+wcet;
					nos_r = i;
				}
			}
		}
	}

#if (SCHED_DBG==1)
	double time_duration = what_time_is_it_now() - start_time;
	printf("scheduling time = %f\n", time_duration);
	printf("Thus, scheduler selects NPU%d\n", nos_r);
#endif

	return nos_r;
}

Scheduler *scheduler_create(void)
{
	Scheduler *scheduler = malloc(sizeof(Scheduler));
	if (!scheduler)
	{
		printf("Error : scheduler cannot be created\n");
		exit(1);
	}

	for (int i=0; i<256; i++)
	{
		scheduler->nnpos[i] = nnpos_create();
	}

	scheduler->npuos_num = 0;

	for (int i=0; i<MAX_NOS_NUM; i++)
	{
		scheduler->nos[i] = NULL;
	}

	scheduler->mnt = scheduler_mnt;
	scheduler->mwct = scheduler_mwct;

	return scheduler;
}

void scheduler_free(Scheduler *scheduler)
{
	if (scheduler)
	{
		for (int i=0; i<256; i++)
		{
			nnpos_free(scheduler->nnpos[i]);
		}

		free(scheduler);
		scheduler = NULL;
	}
}
