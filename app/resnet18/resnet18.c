#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/msg.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include "ntask.h"
#include "nprocess.h"
#include "demo/image.h"
#include "util_time.h"
#include "config.h"
#include "cque.h"

#define DEBUG	0

#define MAX 1000

#ifdef OPENCV
extern char* label_name[1000];
extern Image im[80];
extern char file_name[100][100];
extern Image **alphabet;
extern int preparation(void);

static volatile int demo_done = 0;
static volatile int interrupt_done = 0;

static void ntask_input_handler(Ntask *nt) { }

//static int output_offset, output_size;
static void ntask_output_handler(Ntask *nt) { }

#define CQUE_MAX	4

static Cque *vq;
static Cque *dq;
static volatile int num_send = 0;
static volatile int num_recv = 0;

typedef struct disp_info
{
	int image_index;
	int output_num;
	double time;
} DispInfo;

static Nprocess *nprocess_make(void)
{
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

	np->contain(np, 4, nt0, nt1, nt2, nt3);

	// let the all the affinity to be equal 
	np->set_affinity_unity(np, AFFINITY_UNITY_FORCE);

	return np;
}

static int display_image(int image_num, int output_num, double act_lat)
{
	// fetch answer number
	Image input_im = im[image_num];

	// get label image from the answer number
	Image label = get_label(alphabet, label_name[output_num], (input_im.h*.03));

	// embeds the label image into the original image
	float rgb[3] = {1, 1, 1};
	draw_label(input_im, 220, 0, label, rgb);

	// remove the label for freeing memory
	image_free(label);

#define PREDICTION 1
#if (PREDICTION==1)
	// embeds the time info into the original image
	char timestr[100];

	sprintf(timestr, "%.3fms", act_lat);

	Image label2 = get_label(alphabet, timestr, (input_im.h*.03));
	float rgb2[3] = {.8, .9, .4};
	draw_label(input_im, 0, 80, label2, rgb2);

	image_free(label2);
#endif

	// show the created image here
	int c = show_image(input_im, "predictions", 1);

	return c;
}

void *display_thread(void *args)
{
#if (DEBUG==1)
	printf("display thread begin\n");
#endif
	int prev_seq = -1;

	while (!demo_done && !interrupt_done)
	{
		//printf("wait display thread to receive\n");
repeat:
		while (dq->count==0 && !demo_done);
		if (demo_done) break;

		Cnode *cn = dq->pop_sort(dq, prev_seq);
		int *i;
		int seq;
		if (cn)
		{
			i = cn->value;
			seq = cn->seq;
			prev_seq = seq;
			num_recv++;
		}
		else
		{
			goto repeat;		
		}

#if (DEBUG==1)
		printf("dq(%d): recved value = %d\n", seq, *i);
#endif
		DispInfo *info = (DispInfo *) i;
		int image_index = info->image_index;
		int output_num = info->output_num;
		double time = info->time;
		free(info);

		cnode_free(cn);
#if 0
		printf("image_index, output_num, time = %d %d %f\n", image_index, output_num, time);
#endif
		
		// give input_image, output_number and total_time to display the final output
		int c = display_image(image_index, output_num, time);
		
		if (c != -1)
		{
			c = c%256;
		}
		if (c == 27)  // ESC key; terminal condition
		{
			printf("ESC key pressed!\n");
			interrupt_done = 1;
		}

		//printf("dq->avg_arr_time = %f\n", dq->avg_arr_time);
		//usleep(dq->avg_arr_time*1e6);
	}

	printf("display thread exit\n");

	pthread_exit(NULL);

//#if (DEBUG==1)
//#endif
	return 0;
}

void *job_thread(void *args)
{
#if (DEBUG==1)
	printf("job thread %lu begin\n", pthread_self());
#endif
	Nprocess *np= nprocess_make();

	while (!demo_done)
	{
		//printf("wait job thread to receive\n");
		while (vq->count==0 && !demo_done && !interrupt_done);
		if (demo_done || interrupt_done) break;

		Cnode *cn = vq->pop(vq);
		int *i = cn->value;
		int seq = cn->seq;
#if (DEBUG==1)
		printf("cq: recved value = %d\n", *i);
#endif

		int image_index = *i;

		free(i);
		cnode_free(cn);

		char *image_path = file_name[image_index];
		np->head->set_file_input(np->head, image_path);

		np->run(np);
	
		// get output number & time_took
		int output_offset, output_size;
		np->tail->get_data_output(np->tail, &output_offset, &output_size);
		int *num = (int*)(np->head->get_memory_pointer(np->head)+output_offset);

		double total_time = np->get_time(np)*1e3;

#if (DEBUG==1)
		printf("total_time = %f\n", total_time);
#endif

		DispInfo *info = malloc(sizeof(DispInfo));
		info->image_index = image_index;
		info->output_num = *num;
		info->time = total_time;

		int *value = (int *) info;

		cn = cnode_create();
		cn->value = value;
		cn->seq = seq;
		cn->arr_time = what_time_is_it_now();

		while (dq->push_sort(dq, cn)<0);

#if (DEBUG==1)
		printf("sent %d to dq\n", image_index);
#endif
	}

	np->exit(np);// ntask destroy

	printf("job thread exit\n");

	pthread_exit(NULL);

//#if (DEBUG==1)
//#endif
	return 0;
}

#define MAX_THREAD_N	2

int main(int argc, char *argv[])
{
	int seq = 0;
	preparation();

	pthread_t tid[MAX_THREAD_N];
	for (int i=0; i<MAX_THREAD_N; i++)
	{
		if (pthread_create(&tid[i], 0, job_thread, NULL))
		{
			fprintf(stderr, "thread creation failed\n");
		}
	}

	pthread_t display_tid;
	if (pthread_create(&display_tid, 0, display_thread, NULL))
	{
		fprintf(stderr, "thread creation failed\n");
	}

	vq = cque_create();
	dq = cque_create();

//#if (DEBUG==1)
	double start = what_time_is_it_now();
//#endif
	// infinite loop; i=image_number
	for(int i=0; i<80; i++)
	{
		if (interrupt_done)
			break;
		int *value = malloc(sizeof(int));
		*value = i; // image_number

		Cnode *cn = cnode_create();	
		cn->value = value;
		cn->seq = seq++;

		while (vq->push(vq, cn)<0);

		num_send++;
#if (DEBUG==1)
		printf("sent %d\n", i);
#endif
	}

	// terminates all the threads to make this program done
	while (num_send > num_recv && !interrupt_done); // exit when num_send == num_recv

	demo_done = 1;
	//printf("2. main thread ends\n");

	pthread_join(display_tid, NULL);
	for (int i=0; i<MAX_THREAD_N; i++)
	{
		pthread_join(tid[i], NULL);
	}

	free(vq);
	free(dq);

//#if (DEBUG==1)
	printf("total time = %f\n", (what_time_is_it_now() - start));
//#endif
}
#else
int main(int argc, char *argv[])
{
	fprintf(stderr, "Demo needs OpenCV for mp4 images.\n");
}
#endif
