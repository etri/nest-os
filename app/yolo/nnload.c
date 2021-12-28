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

#define NOSID	0

// usage : nnload yolov2.weights
int main(int argc, char *argv[])
{
	Loader *loader=loader_create();

	if (argc == 1) // argc == 1
	{
		loader->load(loader, MODEL_NAME, NOSID); // install into NPU OS #0
	}
	else if (argc == 2)
	{
		if (!strcmp(argv[1], "-undo"))
		{
			loader->unload(loader, MODEL_NAME, NOSID); // install into NPU OS #0
		}
		else
		{
			//loader->set_file(loader, MODEL_NAME, 0, "yolov2.weights");
			loader->set_file(loader, argv[1]);
			loader->load(loader, MODEL_NAME, NOSID); // install into NPU OS #0
		}
	}
	else if (argc == 3)
	{
		if (!strcmp(argv[1], "-nos"))
		{
			int nos_num = atoi(argv[2]);
			loader->load(loader, MODEL_NAME, nos_num); // install into NPU OS #0
		}
	}
	else
	{
		printf("Usage : nnload\n");
		printf("Usage : nnload <filename>\n");
		printf("Usage : nnload -undo\n");
	}

	//loader->close(loader); // release all the rest resources
}
