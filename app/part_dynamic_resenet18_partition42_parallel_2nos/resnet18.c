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
//#define TRANSFER_DATA_SIZE 401408

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
	Ntask *nt0 = ntask_partition_create("nt0", RESNET18_NPU_DYNAMIC, 0,  ntask_input_handler, OUTPUT_HANDLER_NONE);
	Ntask *nt1 = ntask_partition_create("nt1", RESNET18_NPU_DYNAMIC, 1,  INPUT_HANDLER_NONE, OUTPUT_HANDLER_NONE);
	Ntask *nt2 = ntask_partition_create("nt2", RESNET18_NPU_DYNAMIC, 2,  INPUT_HANDLER_NONE, OUTPUT_HANDLER_NONE);
	Ntask *nt3 = ntask_partition_create("nt3", RESNET18_NPU_DYNAMIC, 3,  INPUT_HANDLER_NONE, OUTPUT_HANDLER_NONE);
	Ntask *nt4 = ntask_partition_create("nt4", RESNET18_NPU_DYNAMIC, 4,  INPUT_HANDLER_NONE, OUTPUT_HANDLER_NONE);
	Ntask *nt5 = ntask_partition_create("nt5", RESNET18_NPU_DYNAMIC, 5,  INPUT_HANDLER_NONE, OUTPUT_HANDLER_NONE);
	Ntask *nt6 = ntask_partition_create("nt6", RESNET18_NPU_DYNAMIC, 6,  INPUT_HANDLER_NONE, OUTPUT_HANDLER_NONE);
	Ntask *nt7 = ntask_partition_create("nt7", RESNET18_NPU_DYNAMIC, 7,  INPUT_HANDLER_NONE, OUTPUT_HANDLER_NONE);
	Ntask *nt8 = ntask_partition_create("nt8", RESNET18_NPU_DYNAMIC, 8,  INPUT_HANDLER_NONE, OUTPUT_HANDLER_NONE);
	Ntask *nt9 = ntask_partition_create("nt9", RESNET18_NPU_DYNAMIC, 9,  INPUT_HANDLER_NONE, ntask_output_handler);

	nt0->next_nt(nt0, nt1);
	nt0->next_nt(nt0, nt2);
	nt1->next_nt(nt1, nt3);
	nt2->next_nt(nt2, nt3);

	nt3->next_nt(nt3, nt4);
	nt3->next_nt(nt3, nt5);
	nt4->next_nt(nt4, nt6);
	nt5->next_nt(nt5, nt6);

	nt6->next_nt(nt6, nt7);
	nt6->next_nt(nt6, nt8);
	nt7->next_nt(nt7, nt9);
	nt8->next_nt(nt8, nt9);

	np->contain(np, 10, nt0, nt1, nt2, nt3, nt4, nt5, nt6, nt7, nt8, nt9);

	nt1->set_affinity(nt1, MASK(0));
	nt2->set_affinity(nt2, MASK(1));
//	nt2->set_priority(nt, 10);

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

		int* num = (int*)(nt9->get_memory_pointer(nt9)+output_offset);
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