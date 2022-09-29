#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "loader.h"
#include "support_model.h"
#include "config.h"

#define PARTITION_0 	0
#define PARTITION_1 	1

#define NOS0	0
#define NOS1	1

int main(int argc, char *argv[])
{
	Loader *loader=loader_create();

	if (argc == 1)
	{
		loader->load_partition(loader, MODEL_NAME, PARTITION_0, NOS0); // install into NPU OS #0
		loader->load_partition(loader, MODEL_NAME, PARTITION_1, NOS1); // install into NPU OS #0
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
