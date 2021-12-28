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
#include <errno.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "os_shm.h"

///////////////////// USAGE ///////////////////
//
// id = shm_create(1, 64*1024*1024);
// void *shmptr = shm_attach(id);
//
// write_something_in_shared_memory(shmptr, "data");
//
// shm_detach(shmptr);
// shm_destroy(id);
//
///////////////////////////////////////////////

int shm_create(int shm_key, int shm_size)
{
	// create shared memory for process communication
        int shmid = shmget(shm_key, shm_size, 0666|IPC_CREAT);
        if (shmid == -1)
        {
                perror("Error: shared memory get failed\n");
        }

	return shmid;
}

void shm_destroy(int shmid)
{
	shmctl(shmid, IPC_RMID, 0);
}

void *shm_attach(int shmid)
{
	void *shmptr = shmat(shmid, (void *)0, 0);

	return shmptr;
}

void shm_detach(void *shmptr)
{
	shmdt(shmptr);
}
