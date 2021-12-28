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
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdarg.h>
#include "nprocess.h"

#define NPROCESS_DEBUG	0

static void *fork_nt(void *args)
{
        Ntask *nt = (Ntask *)args;
        nt->tid = pthread_self();

#if (NPROCESS_DEBUG==1)
        printf("run_nt %s\n", nt->name);
#endif

	if (nt->num_pred==1)
	{
		Ntask *prev_nt = nt->pred[0];	

		if (!prev_nt->output_handler && !nt->input_handler)	
		{
			nt->indata_start_offset = prev_nt->outdata_start_offset;
			nt->indata_size = prev_nt->outdata_size;
			nt->input_type |= INPUT_DATA_BIT;

			nt->msg_predict.indata_start_offset = prev_nt->outdata_start_offset;
			nt->msg_predict.indata_size = prev_nt->outdata_size;
			nt->msg_predict.input_type |= INPUT_DATA_BIT; 
		}
	}

        // execute Ntask for inference 
        nt->run(nt);

        for (int i=0; i<nt->num_succ; i++)
        {
                Ntask *succ_nt = nt->succ[i];

                succ_nt->num_vpred--;

                if (!succ_nt->num_vpred) // run succ_np
                {
                        if (nt->num_vsucc > 1)
                        {
                                pthread_t tid;
                                if (pthread_create(&tid, 0, fork_nt, succ_nt))
                                {
                                        fprintf(stderr, "Thread creation failed\n");
                                        succ_nt->num_vpred++;
                                }
                                else
                                {       // fork successfully
#if (NPROCESS_DEBUG==1)
                                        printf("%s fork %s\n", nt->name, succ_nt->name);
#endif
                                        nt->num_vsucc--;
                                }
                        }
                        else if (nt->num_vsucc == 1)
                        {
                                nt->num_vsucc--;
                                fork_nt(succ_nt);
                        }
#if (NPROCESS_DEBUG==1)
                        else
                        {
                                printf("nt->num_vsucc == 0 : error case\n");
                        }
#endif
                }
        }

        return NULL;
}

static void nprocess_run(Nprocess *np)
{
	// set up for # of pred and succ
        for (Ntask *nt=np->head; nt!=NULL; nt=nt->next)
        {
                nt->num_vpred = nt->num_pred;
                nt->num_vsucc = nt->num_succ;

#if (NPROCESS_DEBUG==1)
                printf("nt name : %s num_vpred = %d num_vsucc = %d \n", nt->name, nt->num_vpred, nt->num_vsucc);
#endif
        }

        np->num_vpred = np->num_pred;
        np->num_vsucc = np->num_succ;

#if (NPROCESS_DEBUG==1)
        printf("np name : %s num_vsucc = %d num_vpred = %d\n", np->name, np->num_vsucc, np->num_vpred);
#endif

	for (int i=0; i<np->num_succ; i++)
        {
                Ntask *succ_nt = np->succ[i];
                //printf("np name : %s\n", succ_np->name);

                if (np->num_vsucc > 1)
                {
                        pthread_t tid;

                        if (pthread_create(&tid, 0, fork_nt, succ_nt))
                        {
                                fprintf(stderr, "Thread creation failed\n");
                                succ_nt->num_vpred++;
                        }
                        else
                        {
                                // fork successfully
#if (NPROCESS_DEBUG==1)
                                printf("%s fork %s\n", np->name, succ_nt->name);
#endif
                                np->num_vsucc--;
                        }
                }
                else if (np->num_vsucc == 1)
                {
                        np->num_vsucc--;
                        fork_nt(succ_nt);
                }
#if (NPROCESS_DEBUG==1)
                else
                {
                        printf("np->num_vsucc == 0 : error case\n");
                }
#endif
        }

        for (int i=0; i<np->num_pred; i++)
        {
                Ntask *pred_nt = np->pred[i];

                while (!pred_nt->tid); // wait until pred_np->tid is determined

                pthread_join(pred_nt->tid, NULL);
        }
}

static void nprocess_exit(Nprocess *np)
{
	Ntask *nt; // p is the sub-nprocess

	for (nt=np->head; nt!=NULL; nt=nt->next)
	{
		nt->exit(nt);
	}

	// free nprocess memory
	free(np);
}

static void nprocess_set_affinity(Nprocess *np, unsigned int mask)
{
	Ntask *nt; // p is the sub-nprocess

	for (nt=np->head; nt!=NULL; nt=nt->next) 
	{ 
		nt->set_affinity(nt, mask);
	}
}

static void nprocess_set_priority(Nprocess *np, int priority)
{
	Ntask *nt; // p is the sub-nprocess

	for (nt=np->head; nt!=NULL; nt=nt->next)
	{
		nt->set_priority(nt, priority);
	}
}

#if 0
static void nprocess_add_ntask(Nprocess *np, Ntask *nt)
{
	if (!np->head)
	{
		np->head = np->tail = nt;
	}
	else
	{
		np->tail->next = nt;
		np->tail = nt;
		np->tail->next = NULL;
	}
	nt->np = np;

	if (nt->memory_pointer)
	{
		nt->free_memory(nt);
	}
	else // task memory uses process memory in common
	{
		nt->memory_pointer = np->memory_pointer;
		nt->memory_size = np->memory_size;
	}
}

static void nprocess_next_np(Nprocess *np, Nprocess *np_next)
{
	np->succ[np->num_succ++] = np_next;
	np_next->pred[np_next->num_pred++] = np;
}
#endif

static void nprocess_contain(Nprocess *np, int args, ...)
{
        va_list ap;

        va_start(ap, args);

        for (int i=0; i<args; i++)
        {
                Ntask *nt = va_arg(ap, Ntask *);

                if (!np->head)
                {
                        np->head = np->tail = nt;
                }
                else
                {
                        np->tail->next = nt;
                        np->tail = nt;
                        np->tail->next = NULL;
                }

#if (NPROCESS_DEBUG==1)
                printf("np_contain => nt_name : %s\n", nt->name);
#endif
		// Nt uses Np's workspace memory
                if (nt->memory_pointer)
                {
                        nt->free_memory(nt);
                }

                nt->memory_pointer = np->memory_pointer;
                nt->memory_size = np->memory_size;

                nt->np = np;
        }

        va_end(ap);

	// set up for # of pred and succ of np
        for (Ntask *nt=np->head; nt!=NULL; nt=nt->next)
        {
                if (!nt->num_pred) // this is head
                {
                        np->succ[np->num_succ++] = nt;
                }

                if (!nt->num_succ) // this is tail
                {
                        np->pred[np->num_pred++] = nt;
                }
        }
}

Nprocess *nprocess_create(char *name)
{
	Nprocess *nprocess;

	nprocess = malloc(sizeof(Nprocess));
	if (!nprocess)
	{
		printf("Error: nprocess cannot be created\n");
		exit(1);
	}

	strcpy(nprocess->name, name);

	nprocess->head = nprocess->tail = NULL;

	nprocess->num_pred = 0;
	nprocess->num_succ = 0;
	nprocess->num_vpred = 0;
	nprocess->num_vsucc = 0;

	for (int i=0; i<MAX_PRED_NUM; i++)
	{
		nprocess->pred[i] = NULL;
	}

	for (int i=0; i<MAX_SUCC_NUM; i++)
	{
		nprocess->succ[i] = NULL;
	}

	for (int i=0; i<MAX_NUMTHREAD; i++)
	{
		nprocess->thread[i] = (pthread_t) NULL;
	}

	nprocess->set_affinity = nprocess_set_affinity;
	nprocess->set_priority = nprocess_set_priority;
	nprocess->run = nprocess_run;
	nprocess->exit = nprocess_exit;
	nprocess->contain = nprocess_contain;

	// workspace memory attached
        nprocess->memory_pointer = malloc(NTASK_WSMEM_SIZE);
        if (nprocess->memory_pointer==NULL)
        {
                printf("Error : memory allocation failed\n");
                exit(1);
        }
        nprocess->memory_size = NTASK_WSMEM_SIZE; // 64MBytes

	return nprocess;
}

