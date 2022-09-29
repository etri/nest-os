#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "loader.h"
#include "support_model.h"
#include "config.h"

int main(int argc, char *argv[])
{
	Loader *loader=loader_create();

	if (argc == 1)
	{
		loader->load_partition_range(loader, MODEL_PART_DYNAMIC_NAME, 0, 17, 0); // install into NPU OS #0
		printf("MODEL NAME %d\n", MODEL_PART_DYNAMIC_NAME);
	}
	else if (argc == 2)
	{
		if (!strcmp(argv[1], "-undo"))
		{
			loader->unload_partition_range(loader, MODEL_PART_DYNAMIC_NAME, 0, 17, 0); // uninstall into NPU OS #0
		}
	}
}
