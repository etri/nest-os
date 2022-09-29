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
#ifndef NPROCESS_H
#define NPROCESS_H

#include <pthread.h>
#include "ntask.h"

#define MAX_NAME_LENGTH	20
#define MAX_NUMTHREAD 100

typedef struct nprocess
{
	char name[MAX_NAME_LENGTH];
	void *memory_pointer;
	int memory_size;
	struct timeval time;

	Ntask *head;
	Ntask *tail;

	pthread_t thread[MAX_NUMTHREAD];
	int num_pred;
	int num_succ;
	int num_vpred;
	int num_vsucc;
	pthread_mutex_t num_vpred_lock;
        pthread_mutex_t num_vsucc_lock;

	Ntask *pred[MAX_PRED_NUM];
	Ntask *succ[MAX_SUCC_NUM];

        void (*set_affinity)(struct nprocess *np, unsigned int mask);
	void (*set_priority)(struct nprocess *np, int priority);
   	void (*run)(struct nprocess *np);
        void (*exit)(struct nprocess *np);
	void (*contain)(struct nprocess *np, int args, ...);
	double (*get_time)(struct nprocess *np);
} Nprocess;

Nprocess *nprocess_create(char *name);
#endif
