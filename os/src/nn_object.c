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
#include "nn_object.h"

NnObject *nn_object_create(void)
{
        NnObject *nn_object;

        nn_object = malloc(sizeof(NnObject));
	
	if (!nn_object)
	{
		printf("nn_object_create: insufficient memory\n");
		exit(1);
	}

        nn_object->nnid = 0;
        nn_object->wsmem = NULL;
        nn_object->loaded = 0;
        nn_object->npu = NULL;
	nn_object->load_filename_buf = NULL;
#if 0
	for (int i=0; i<4; i++)
		nn_object->part_bit[i] = 0;
#else
	for (int i=0; i<BIT_STRING_NUM; i++)
	{
		nn_object->bs.bit[i] = 0;
	}	
#endif
	//nn_object->in_filename = filename_buf;

        nn_object->load = NULL;
        nn_object->unload = NULL;
        nn_object->predict = NULL;
        nn_object->set_input = NULL;
        nn_object->get_output = NULL;

  	return nn_object;
}

void nn_object_free(NnObject *nn_object)
{
	if (nn_object->load_filename_buf)
	{
		free(nn_object->load_filename_buf);
		nn_object->load_filename_buf = NULL;
	}
        free(nn_object);
}

