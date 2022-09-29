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
#include "util_time.h"

#define NPROCESS_DEBUG	0

static void *fork_nt(void *args)
{
        Ntask *nt = (Ntask *)args;
        nt->tid = pthread_self();

#if (NPROCESS_DEBUG==1)
        printf("run_nt %s\n", nt->name);
#endif

	if (nt->num_pred)
	{
		nt->indata_start_offset = 0;
		nt->indata_size = 0;
		nt->input_type |= INPUT_DATA_BIT;

		for (int i=0; i<nt->num_pred; i++) // for each predecessor
                {
                	Ntask *prev_nt = nt->pred[i];

                        nt->indata_size += prev_nt->outdata_size;
                }

                nt->msg_predict.indata_start_offset_remote = 0; // automatically 0, if needed, change it in the future (TO DO)
                nt->msg_predict.indata_size = nt->indata_size;
                nt->msg_predict.input_type |= INPUT_DATA_BIT;
	}

        // execute Ntask for inference 
        nt->run(nt);

#if (NPROCESS_DEBUG==1)
        printf("run_nt %s finished\n", nt->name);
#endif
        for (int i=0; i<nt->num_succ; i++)
        {
                Ntask *succ_nt = nt->succ[i];

		pthread_mutex_lock(&succ_nt->num_vpred_lock);
                succ_nt->num_vpred--;

                if (!succ_nt->num_vpred) // run succ_np
                {
			pthread_mutex_lock(&nt->num_vsucc_lock);
                        if (nt->num_vsucc > 1)
                        {
                                pthread_t tid;
                                if (pthread_create(&tid, 0, fork_nt, succ_nt))
                                {
					pthread_mutex_unlock(&nt->num_vsucc_lock);

                                        succ_nt->num_vpred++;
					pthread_mutex_unlock(&succ_nt->num_vpred_lock);

                                        fprintf(stderr, "Thread creation failed\n");
                                }
                                else
                                {       // fork successfully
					pthread_mutex_unlock(&succ_nt->num_vpred_lock);

                                        nt->num_vsucc--;

					pthread_mutex_unlock(&nt->num_vsucc_lock);
#if (NPROCESS_DEBUG==1)
                                        printf("%s fork %s\n", nt->name, succ_nt->name);
#endif
                                }
                        }
                        else if (nt->num_vsucc == 1)
                        {
                                nt->num_vsucc--;
				pthread_mutex_unlock(&nt->num_vsucc_lock);
				pthread_mutex_unlock(&succ_nt->num_vpred_lock);

                                fork_nt(succ_nt);
                        }
#if (NPROCESS_DEBUG==1)
                        else
                        {
				pthread_mutex_unlock(&nt->num_vsucc_lock);
				pthread_mutex_unlock(&succ_nt->num_vpred_lock);
                                printf("nt->num_vsucc == 0 : error case\n");
                        }
#endif
                }
		else
		{
			pthread_mutex_unlock(&succ_nt->num_vpred_lock);
		}
        }

        return NULL;
}

static void nprocess_run(Nprocess *np)
{
	struct timeval t1 = what_time(); // measure time

	// set up for # of pred and succ
        for (Ntask *nt=np->head; nt!=NULL; nt=nt->next)
        {
                nt->num_vpred = nt->num_pred;
                nt->num_vsucc = nt->num_succ;
		nt->tid = 0; // make sure that all tid's are initially zero

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

		pthread_mutex_lock(&np->num_vsucc_lock);
                if (np->num_vsucc > 1) // multiple successor -> use pthread_fork(fork_nt())
                {
                        pthread_t tid;

                        int ret = pthread_create(&tid, 0, fork_nt, succ_nt);

			if (ret) // ret contains error number
                        {
                                fprintf(stderr, "Thread creation failed\n");
				pthread_mutex_unlock(&np->num_vsucc_lock);
                        }
                        else // fork successfully
                        {
                                np->num_vsucc--;
				pthread_mutex_unlock(&np->num_vsucc_lock);

				pthread_mutex_lock(&succ_nt->num_vpred_lock);
                		succ_nt->num_vpred--;
				pthread_mutex_unlock(&succ_nt->num_vpred_lock);
#if (NPROCESS_DEBUG==1)
                                printf("%s fork %s\n", np->name, succ_nt->name);
#endif
                        }
                }
                else if (np->num_vsucc == 1) // single successor -> use function call(fork_nt())
                {
                        np->num_vsucc--;
			pthread_mutex_unlock(&np->num_vsucc_lock);

                        fork_nt(succ_nt);
                }
#if (NPROCESS_DEBUG==1)
                else
                {
			pthread_mutex_unlock(&np->num_vsucc_lock);
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

	struct timeval t2 = what_time(); // measure time
	np->time = what_time_diff(t1, t2); // calculate t2-t1
}

static double nprocess_get_time(Nprocess *np)
{
        return(timeval_to_double(np->time));
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

	pthread_mutex_init(&nprocess->num_vpred_lock, NULL);
        pthread_mutex_init(&nprocess->num_vsucc_lock, NULL);

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
	nprocess->get_time = nprocess_get_time;

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

