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

	loader->load(loader, MODEL_NAME, 0); // install into NPU OS #0
//	loader->load(loader, MODEL_NAME, 1); // install into NPU OS #1
}
