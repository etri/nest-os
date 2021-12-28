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
#ifndef OS_SHM_H
#define OS_SHM_H

#include <sys/types.h>

typedef struct freelist
{
        size_t sz;
        struct freelist *nx;
} Freelist;

typedef struct mem
{
        size_t *heap_start_addr;
        size_t *heap_end_addr;
        Freelist *flp;       // freelist pointer (head of freelist)
        char *brkval;    // first location not yet allocated. use for making new chunk
        void (*init)(struct mem *m, void *heap_start, size_t size);
        void *(*malloc)(struct mem *m, size_t len);
        void *(*calloc)(struct mem *m, size_t num, size_t len);
        void (*free)(struct mem *m, void *p);
} Mem;

Mem *mem_create(void *m_start, size_t size);
void mem_free(Mem *m);

int shm_create(int shm_key, int shm_size);
void shm_destroy(int shmid);

void *shm_attach(int shmid);
void shm_detach(void *shmptr);

#endif
