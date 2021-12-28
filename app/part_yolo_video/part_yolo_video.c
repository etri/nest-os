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
#include "ntask.h"
#include "nprocess.h"
#include "demo/image.h"
#include "util_time.h"
#include "config.h"

#ifdef OPENCV
#define DIVIDER 2

static void *cap;

static void ntask_input_handler(Ntask *nt)
{
#define TRANSFER_DATA_SIZE 346176

	// 1. prepare image and prediction
	Image im = get_image_from_stream(cap);

	// 2. image copy into shared memory region
	Image *im_in = (Image *)(nt->get_memory_pointer(nt) + TRANSFER_DATA_SIZE);
	*im_in = im;
	im_in->data = (float *)(nt->get_memory_pointer(nt)+ TRANSFER_DATA_SIZE + 32);
	memcpy(im_in->data, im.data, im.h*im.w*im.c*sizeof(float));

	// 3. specify input and output data region in memory
	nt->set_data_input(nt, TRANSFER_DATA_SIZE, 32+im.h*im.w*im.c*sizeof(float));
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

static int inference_and_show(Nprocess *np)
{
        // fetch & predict & print time took
        double time1=what_time_is_it_now();
        np->run(np);
        double time2=what_time_is_it_now();
        printf("Prediction: %f secs\n", time2 - time1);

        // display the output
	Ntask *nt2 = np->tail;
	int output_offset, output_size;
	nt2->get_data_output(nt2, &output_offset, &output_size);
	Image *im_out = (Image *)(nt2->get_memory_pointer(nt2)+output_offset);
        int c = show_image(*im_out, "Demo", 1);
        if (c != -1)
                c = c%256; // for AscII ouput (0~256)
        if (c == 27)  // ESC key; terminal condition
                return 1; // demo terminate
        else
                return 0;
}

#define PARTITION_0     0
#define PARTITION_1     1

int main(int argc, char *argv[])
{
        // for input data
        char *datapath;
        datapath = (argv[1]==NULL)? "data/BandVideo.mp4": argv[1];
//      datapath = (argv[1]==NULL)? 0 : argv[1]; // for camera
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

	Nprocess *np = nprocess_create("np");

	// ntask create and do setting if necessary (affinity or priority)
	Ntask *nt1 = ntask_partition_create("nt1", MODEL_NAME, PARTITION_0, ntask_input_handler, OUTPUT_HANDLER_NONE);
        Ntask *nt2 = ntask_partition_create("nt2", MODEL_NAME, PARTITION_1, INPUT_HANDLER_NONE, ntask_output_handler);
	
	//np->add_ntask(np, nt1); // link: nt1->nt2
	//np->add_ntask(np, nt2); // link: nt1->nt2
        nt1->next_nt(nt1, nt2);

	np->contain(np, 2, nt1, nt2);

	nt1->set_affinity(nt1, MASK(0));
	nt2->set_affinity(nt2, MASK(1));
	//nt->set_priority(nt, 10);

	//nt1->set_partition(nt1, 0, 0);
	//nt2->set_partition(nt2, 1, 1);

        // infinite loop
        int demo_done = 0;
        while (!demo_done)
        {
                demo_done = inference_and_show(np);
        }

        np->exit(np);
}
#else
int main(int argc, char *argv[])
{
    fprintf(stderr, "Demo needs OpenCV for webcam images.\n");
}
#endif
