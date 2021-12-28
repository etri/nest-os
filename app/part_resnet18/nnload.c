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
#include "error.h"
#include "loader.h"
#include "support_model.h"
#include "config.h"

#define PARTITION_0     0
#define PARTITION_1     1

#define NOS0     0
#define NOS1     1

int main(int argc, char *argv[])
{
	Loader *loader=loader_create();

	if (argc == 1)
	{
		loader->load_partition(loader, MODEL_NAME, PARTITION_0, NOS0); // install into NPU OS #0
        	loader->load_partition(loader, MODEL_NAME, PARTITION_1, NOS1); // install into NPU OS #1
	}
	else if (argc == 2)
	{
		if (!strcmp(argv[1], "-undo"))
                {
                        loader->unload_partition(loader, MODEL_NAME, PARTITION_0, NOS0); // install into NPU OS #0
                        loader->unload_partition(loader, MODEL_NAME, PARTITION_1, NOS1); // install into NPU OS #0
                }
	}
}
