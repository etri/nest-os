#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "ntask.h"
#include "demo/image.h"
#include "util_time.h"
#include "config.h"

#ifdef OPENCV

#define DIVIDER 2

static void *cap;
static Image im;

static void ntask_input_handler(Ntask *nt)
{
        // 1. image copy into shared memory region
        Image *im_in = nt->get_memory_pointer(nt);
        *im_in = im;
        im_in->data = (float *)(nt->get_memory_pointer(nt)+32);
        memcpy(im_in->data, im.data, im.h*im.w*im.c*sizeof(float));

        // 2. specify input and output data region in memory
        nt->set_data_input(nt, 0, 32+im.h*im.w*im.c*sizeof(float));
}

static void ntask_output_handler(Ntask *nt)
{
	int output_offset, output_size;

	nt->get_data_output(nt, &output_offset, &output_size);
        //printf("output data : offset = %d size = %d\n", output_offset, output_size);

        // get the output data, and adjust the image data pointer if necessary
        Image *im_out = (Image *)(nt->get_memory_pointer(nt)+output_offset);
        im_out->data = (float *)(nt->get_memory_pointer(nt)+output_offset+32);
}

static int inference_and_show(Ntask *nt)
{
	// fetch & predict & print time took
	// fetch image from cap
    	im = get_image_from_stream(cap);
	if (im.data == 0)
	{
		printf("This video frame is the end\n");
		return 1; // demo terminated
	}

    	//double time1=what_time_is_it_now();
    	nt->run(nt);
    	//double time2=what_time_is_it_now();
    	//printf("Prediction: %f secs\n", time2 - time1);
    	printf("Prediction: %f secs mask: %x\n", nt->get_time(nt), nt->get_nos_mask(nt));

       	// display the output
       	Image *im_out = (Image *)(nt->get_memory_pointer(nt)+nt->outdata_start_offset);
	int c = show_image(*im_out, "Demo", 1);
    	if (c != -1) 
	{
		c = c%256; // for AscII ouput (0~256)
	}

    	if (c == 27)  // ESC key; terminal condition
	{
		printf("ESC key pressed!\n");
       		return 1; // demo terminate
	}

	return 0;
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

	// for display
	int fullscreen = 0;
        make_window("Demo", 1352/DIVIDER, 1013/DIVIDER, fullscreen);

	Ntask *nt = ntask_create("nt", MODEL_NAME, ntask_input_handler, ntask_output_handler);
	//nt->set_affinity(nt, MASK(1)); // for NPU affinity

	// infinite loop
	volatile int demo_done = 0;
    	while (!demo_done)
    	{
		demo_done = inference_and_show(nt);
    	}

	nt->exit(nt);

	getchar();
}
#else
int main(int argc, char *argv[])
{
    fprintf(stderr, "Demo needs OpenCV for webcam images.\n");
}
#endif

