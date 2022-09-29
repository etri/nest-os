#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <math.h>
#include "ntask.h"
#include "nprocess.h"
#include "demo/image.h"
#include "util_time.h"
#include "config.h"

static void get_answer_labels(char *names[])
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

#define MAX 1000

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

/*
int file_size;
static char *image_path;
static void ntask_input_handler(Ntask *nt)
{

	nt->set_file_input(nt, image_path);
	file_size = nt->infile_size;
}

int output_offset, output_size;
static void ntask_output_handler(Ntask *nt)
{
	nt->get_data_output(nt, &output_offset, &output_size);
	//printf("output data : offset = %d size = %d\n", output_offset, output_size);

}
*/

char* label_name[1000];
Image im[80];
char file_name[100][100];
Image **alphabet;

int preparation(void)
{
	// 1. preparation for answer labels for number output
	get_answer_labels(label_name);

	// 2. preparation for getting 80 images from files in dir 100 to get im[0-79] & filename[]

	DIR *dir;
	struct dirent *ent;
	dir = opendir ("100/");

	if (dir != NULL) 
	{
		int i = 0;
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
			//printf ("%s\n",file_name[i]);
			i++;
		}
		closedir (dir);
	}
	else {
	/* could not open directory */
		perror ("");
		return EXIT_FAILURE;
	}

	for (int i = 0; i < 80; i++)
	{
		im[i] = load_image_color(file_name[i], 224, 224);
	}

	// 3. preparation for drawing alphabet within image
	alphabet = NULL;
	if (!alphabet)
		alphabet = load_alphabet();

	// 4. preparation for image window
	make_window("predictions", 448, 448, 0); // create 2 threads, total 3

	return 0;
}

/*
int main(int argc, char *argv[])
{
	preparation();

// preparation of single neural process
	Nprocess *np = nprocess_create("np");

	// ntask create and do setting if necessary (affinity or priority)
	Ntask *nt0 = ntask_partition_create("nt0", RESNET18_NPU_DYNAMIC, 0, ntask_input_handler, OUTPUT_HANDLER_NONE);
	Ntask *nt1 = ntask_partition_create("nt1", RESNET18_NPU_DYNAMIC, 1, INPUT_HANDLER_NONE, OUTPUT_HANDLER_NONE);
	Ntask *nt2 = ntask_partition_create("nt2", RESNET18_NPU_DYNAMIC, 2, INPUT_HANDLER_NONE, OUTPUT_HANDLER_NONE);
	Ntask *nt3 = ntask_partition_create("nt3", RESNET18_NPU_DYNAMIC, 3, INPUT_HANDLER_NONE, ntask_output_handler);

	nt0->next_nt(nt0, nt1);
	nt0->next_nt(nt0, nt2);
	
	nt1->next_nt(nt1, nt3);
	nt2->next_nt(nt2, nt3);

	nt0->set_affinity(nt0, MASK(0));
	nt1->set_affinity(nt1, MASK(0));
	nt2->set_affinity(nt2, MASK(0));
	nt3->set_affinity(nt3, MASK(0));

	np->contain(np, 4, nt0, nt1, nt2, nt3);

loop_start:
	// infinite loop
	for(int j = 0; j < 80; j++)
	{
		image_path = file_name[j];

		np->run(np);
		
		int *num = (int*)(nt0->get_memory_pointer(nt0)+output_offset);

		Image label = get_label(alphabet, label_name[*num], (im[j].h*.03));
		float rgb[3] = {1, 1, 1};
		draw_label(im[j], 220, 0, label, rgb);
		image_free(label);

#define PREDICTION 1
#if (PREDICTION==1)
		double act_lat = np->get_time(np)*1e3;
		char timestr[100];

		sprintf(timestr, "%.3fms", act_lat);

		Image label2 = get_label(alphabet, timestr, (im[j].h*.03));
		float rgb2[3] = {.8, .9, .4};
		draw_label(im[j], 0, 80, label2, rgb2);
		image_free(label2);
#endif

#if 1
		int c = show_image(im[j], "predictions", 1);

		if (c != -1)
		{
			c = c%256;
		}
		if (c == 27)  // ESC key; terminal condition
		{
			printf("ESC key pressed!\n");
			goto out;
		}
#endif
	}
	goto loop_start;
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
*/
