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
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include "ntask.h"
#include "demo/image.h"
#include "util_time.h"
#include "config.h"

#ifdef OPENCV

#define DIVIDER 2

static void *cap;
// infinite loop
volatile int demo_done = 0;
static pthread_mutex_t fetch_lock = PTHREAD_MUTEX_INITIALIZER;
static double avgtime = 0;
static double curtime = 0;
static unsigned int fetch_n = 0;

#define IMAGEQ_LENGTH	10

typedef struct image_q
{
	pthread_mutex_t lock;
	unsigned int head, tail;
	Image im[IMAGEQ_LENGTH];
} ImageQ;

static ImageQ imq;

static void imageq_push(ImageQ *iq, Image *imp)
{
	unsigned int next_tail = (iq->tail+1)%IMAGEQ_LENGTH;

	if (iq->head == next_tail) // queue is full
	{
		printf("Error. TaskQ is full\n");
		exit(1);
	}
	else
	{
		iq->im[iq->tail] = *imp;
		iq->tail = next_tail;
	}
}

static void imageq_pop(ImageQ *iq)
{
	if (iq->head == iq->tail) // in case of empty
	{
		printf("Error. TaskQ is empty\n");
                exit(1);
	}
	else
        {
		// free image data
		free(iq->im[iq->head].data);
        	iq->head = (iq->head+1)%IMAGEQ_LENGTH; // update tq->head
        }
}

static Image fetch_in_thread(void)
{

        pthread_mutex_lock(&fetch_lock);
    	if (curtime)
	{
		double difftime = what_time_is_it_now() - curtime;

		avgtime = ((fetch_n-1)*avgtime + difftime)/fetch_n;
		printf("avg fetch time: %f secs\n", avgtime);
	}
	else
	{
		printf("avg fetch no time: %f secs\n", avgtime);
	}

        Image im = get_image_from_stream(cap);

	if (fetch_n == IMAGEQ_LENGTH)
	{
		fetch_n = 0;
		avgtime = 0;
	}

	fetch_n++;

    	curtime=what_time_is_it_now();
        pthread_mutex_unlock(&fetch_lock);

	return im;
}

static void *display_in_thread(Image *imp)
{
    	int c = show_image(*imp, "Demo", 1);

    	if (c != -1) c = c%256;
    	if (c == 27)  // ESC key
    	{
		printf("ESC key pressed!\n");
       		demo_done = 1;
    	}

    	return 0;
}

static void ntask_input_handler(Ntask *nt)
{
	// fetch image from cap
	Image im = fetch_in_thread();

	if (im.data == 0)
	{
		printf("This video frame is the end\n");
	}
	else
	{
        	// 1. image copy into shared memory region
        	Image *im_in = nt->get_memory_pointer(nt);
        	*im_in = im;
        	im_in->data = (float *)(nt->get_memory_pointer(nt)+32);
        	memcpy(im_in->data, im.data, im.h*im.w*im.c*sizeof(float));

        	// 2. specify input and output data region in memory
        	nt->set_data_input(nt, 0, 32+im.h*im.w*im.c*sizeof(float));
        	//nt->set_data_output(nt, 0, 32+im.h*im.w*im.c*sizeof(float));
	}
}

int output_offset, output_size;
static void ntask_output_handler(Ntask *nt)
{
	nt->get_data_output(nt, &output_offset, &output_size);
        //printf("output data : offset = %d size = %d\n", output_offset, output_size);

        // get the output data, and adjust the image data pointer if necessary
        Image *im_out = (Image *)(nt->get_memory_pointer(nt)+output_offset);
	float *data = (float *)(nt->get_memory_pointer(nt)+output_offset+32);
        im_out->data = malloc(im_out->h*im_out->w*im_out->c*sizeof(float));
	memcpy(im_out->data, data, im_out->h*im_out->w*im_out->c*sizeof(float));

	// display the output
	//display_in_thread(im_out);
	pthread_mutex_lock(&imq.lock);
	imageq_push(&imq, im_out);
	pthread_mutex_unlock(&imq.lock);
}

void *inference(void *args)
{
	Ntask *nt = (Ntask *) args;

	while (!demo_done)
    	{
    		nt->run(nt);
    	}

	return NULL;
}

void *show_video(void *args)
{
	//printf("show_video\n");
	while (!demo_done)
    	{
		pthread_mutex_lock(&imq.lock);
		while (imq.head != imq.tail) // not empty
		{
			//printf("imq.head = %d imq.tail = %d\n", imq.head, imq.tail);
			Image *imp = &(imq.im[imq.head]);
			display_in_thread(imp);
			imageq_pop(&imq);
			pthread_mutex_unlock(&imq.lock);

			usleep((useconds_t) 1e6*avgtime);

			pthread_mutex_lock(&imq.lock);
		}
		pthread_mutex_unlock(&imq.lock);
    	}
	printf("show_video finish\n");

	return NULL;
}

int main(int argc, char *argv[])
{
	// for input data
	char *datapath;
	datapath = (argv[1]==NULL)? "data/BandVideo.mp4": argv[1];
//	datapath = (argv[1]==NULL)? 0 : argv[1]; // for camera
        cap = open_video_stream(datapath, 0, 0, 0, 0);
    	if (!cap) 
	{
		printf("Couldn't connect to webcam.\n");
		exit(1);
	}
	else
	{
        	printf("video file: %s\n", datapath);
	}

	imq.head = imq.tail = 0;
	pthread_mutex_init(&imq.lock, NULL);

	// for display
	int fullscreen = 0;
        make_window("Demo", 1352/DIVIDER, 1013/DIVIDER, fullscreen);

	Ntask *nt1 = ntask_create("nt1", MODEL_NAME, ntask_input_handler, ntask_output_handler);
	Ntask *nt2 = ntask_create("nt2", MODEL_NAME, ntask_input_handler, ntask_output_handler);
	//nt->affinity(nt, MASK(1)); // for NPU affinity

	pthread_t thread1;
	pthread_t thread2;
	pthread_t thread3;

        if (pthread_create(&thread1, 0, inference, nt1))
                fprintf(stderr, "Thread1 creation failed\n");
        if (pthread_create(&thread2, 0, inference, nt2))
                fprintf(stderr, "Thread2 creation failed\n");
        if (pthread_create(&thread3, 0, show_video, NULL))
                fprintf(stderr, "Thread3 creation failed\n");

        pthread_join(thread1, 0);
        pthread_join(thread2, 0);
        pthread_join(thread3, 0);

	nt1->exit(nt1);
	nt2->exit(nt2);

	getchar();
}
#else
int main(int argc, char *argv[])
{
    fprintf(stderr, "Demo needs OpenCV for webcam images.\n");
}
#endif

