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
#ifndef SCHEDULER_H
#define SCHEDULER_H

#define MAX_NOS_NUM	32
#include "nos_bit.h"
#include "server_npuos.h"

typedef struct scheduler
{
	NnPos *nnpos[256];
	int npuos_num;
        ServerNpuOS *nos[MAX_NOS_NUM];

	int (*function)(struct scheduler *sched, int nnid, int part_num_start, int part_num_delta, unsigned int affinity_mask);

	int (*mnt)(struct scheduler *sched, int nnid, int part_num_start, int part_num_delta, unsigned int affinity_mask);
	int (*mwct)(struct scheduler *sched, int nnid, int part_num_start, int part_num_delta, unsigned int affinity_mask);
} Scheduler;

Scheduler *scheduler_create(void);
void scheduler_free(Scheduler *scheduler);

#endif
