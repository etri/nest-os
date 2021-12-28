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
#include <dirent.h>
#include "ntask.h"
#include "nprocess.h"
#include "demo/image.h"
#include "util_time.h"
#include "config.h"

#define MAX 1000

#ifdef OPENCV

static volatile int demo_done = 0;
Image resized_im;
static char *image_path;

void get_labels(char *names[])
{
	char *label = NULL;

	FILE *fp;
	char *path = "labels.txt";
	int i = 0;

	fp = fopen(path, "r");

	if (fp == NULL)
	{
		fprintf(stderr, "File Open Error!\n");
		exit(1);
	}

	while (!feof(fp))
	{
		label = (char*)malloc(sizeof(char) * MAX);
		fgets(label, MAX, fp);
		names[i] = label;
		i++;
	}

	if (label)
		free(label);
	fclose(fp);
}

void print_label(char *names[], int num)
{
	printf("label : %s\n", names[num]);
}

static void ntask_input_handler(Ntask *nt)
{
	nt->set_file_input(nt, image_path);
}

int output_offset, output_size;
static void ntask_output_handler(Ntask *nt)
{
	// get the output data, and adjust the image data pointer if necessary
	nt->get_data_output(nt, &output_offset, &output_size);
        printf("output data : offset = %d size = %d\n", output_offset, output_size);
}

int main(int argc, char *argv[])
{

	char* name[1000];
	char file_name[100][100];
	int i = 0;

	Image **alphabet = NULL;
	if (!alphabet)
		alphabet = load_alphabet();

	DIR *dir;
	struct dirent *ent;
	dir = opendir ("100/");

	if (dir != NULL) {
	/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL)
	{
		char *s1 = ".";
		char *s2 = "..";
		char str1[100] = "100/";
		char* str2 = ent->d_name;

		if(!strcmp(str2, s1) || !strcmp(str2, s2))
		{
			continue;
		}

		strcat(str1, str2);
		strcpy(file_name[i], str1);
		printf ("%s\n",file_name[i]);
		i++;
		}
		closedir (dir);
	}
	else {
	/* could not open directory */
		perror ("");
		return EXIT_FAILURE;
	}

	// get labels
	get_labels(name);

	Nprocess *np = nprocess_create("np");

	// ntask create and do setting if necessary (affinity or priority)
	Ntask *nt1 = ntask_partition_range_create("nt1", RESNET18_NPU_DYNAMIC, 0, 2, ntask_input_handler, OUTPUT_HANDLER_NONE);
	Ntask *nt2 = ntask_partition_range_create("nt2", RESNET18_NPU_DYNAMIC, 3, 17, INPUT_HANDLER_NONE, ntask_output_handler);

	nt1->next_nt(nt1, nt2);

        np->contain(np, 2, nt1, nt2);

	//np->add_ntask(np, nt1); // link: nt1->nt2
	//np->add_ntask(np, nt2); // link: nt1->nt2

	//nt->set_affinity(nt, MASK(1));
	//nt->set_priority(nt, 10);

	make_window("predictions", 448, 448, 0);

	Image label;
loop:
	// infinite loop
	for(int j = 0; j < 80; j++)
	{
		image_path = file_name[j];
		Image im = load_image_color(file_name[j], 224, 224);

		printf("> prediction starts\n");
		double time=what_time_is_it_now();

		// run input_handler+prediction is made
		np->run(np);

		int* num = (int*)(nt2->get_memory_pointer(nt2)+output_offset);
		printf("> prediction ends -> took %f seconds\n", what_time_is_it_now() - time);

		print_label(name, *num);

		label = get_label(alphabet, name[*num], (im.h*.03));

		float rgb[3] = {1, 1, 1};
		draw_label(im, 220, 0, label, rgb);
		int c = show_image(im, "predictions", 1);

		if (c != -1)
		{
			c = c%256;
		}
		//print_label(name, *num);
		image_free(im);
		image_free(label);

		if (c == 27)  // ESC key; terminal condition
		{
			printf("ESC key pressed!\n");
			goto out;
		}
	}
	goto loop;
out:
#ifdef OPENCV
#endif
	np->exit(np);// ntask destroy
}
#else
int main(int argc, char *argv[])
{
	fprintf(stderr, "Demo needs OpenCV for mp4 images.\n");
}
#endif
