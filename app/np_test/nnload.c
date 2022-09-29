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
