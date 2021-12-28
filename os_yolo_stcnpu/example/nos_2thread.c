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
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "npuos.h"
#include "npu.h"

#define	NUM_THREAD 2

void *hw[2] = {&stc0, &stc1};
//void *hw[2] = {NULL, NULL};

void *os_run(void *arg)
{
	int num = *((int *) arg);
	printf("num = %d\n", num);

	NpuOS *nos;

	nos = npuos_create();

        nos->set_id(nos, num); // id = 0
        nos->set_hwtype(nos, ETRI_STC);
        //nos->set_inf_type(nos, INF_IPC);
        nos->set_inf_type(nos, INF_SOCK);
	nos->set_npu(nos, hw[num]);

	nos->start(nos); // start the os (including init)

	npuos_destroy(nos);

	return NULL;
}	

int main(int argc, char *argv[])
{
	stcnpu_open(); // initialize NPU hardware before OS starts

	pthread_t thread[NUM_THREAD];
	int arg[NUM_THREAD];
	
	for (int i=0; i<NUM_THREAD; i++)
	{
		arg[i] = i;
		if (pthread_create(&thread[i], 0, os_run, &arg[i]))
			fprintf(stderr, "Thread1 creation failed\n");
	}

	// join all pthreads
	for (int i=0; i<NUM_THREAD; i++)
	{
		pthread_join(thread[i], 0);
	}

	stcnpu_close(); // close NPU hardware
}
