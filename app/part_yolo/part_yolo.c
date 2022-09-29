#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ntask.h"
#include "nprocess.h"
#include "util_time.h"
#include "demo/image.h"
#include "config.h"

static char *image_path;

#define FILE_INPUT	0

/////////////////////////////////////////////////
static void ntask_input_handler(Ntask *nt)
{
#if (FILE_INPUT==1)
	nt->set_file_input(nt, image_path);
#else
#define TRANSFER_DATA_SIZE 346176
	// 1. prepare image and prediction
	Image im = load_image_color(image_path, 0, 0);

        // 2. image copy into shared memory region
        Image *im_in = (Image *) (nt->get_memory_pointer(nt));
        *im_in = im;
        im_in->data = (float *)(nt->get_memory_pointer(nt)+32);
        memcpy(im_in->data, im.data, im.h*im.w*im.c*sizeof(float));

        // 3. specify input and output data region in memory
        nt->set_data_input(nt, 0, 32+im.h*im.w*im.c*sizeof(float));
	nt->set_data_input_offset_remote(nt, TRANSFER_DATA_SIZE);
#endif
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
/////////////////////////////////////////////////

#define PARTITION_0 	0
#define PARTITION_1 	1

int main(int argc, char *argv[])
{
	image_path = argc==2? argv[1]:"data/dog.jpg";
	printf("argc = %d argv[1]=%s\n", argc, argv[1]);

	Nprocess *np = nprocess_create("np");

	// ntask create and do setting if necessary (affinity or priority)
	Ntask *nt1 = ntask_partition_create("nt1", MODEL_NAME, PARTITION_0, ntask_input_handler, OUTPUT_HANDLER_NONE);
	Ntask *nt2 = ntask_partition_create("nt2", MODEL_NAME, PARTITION_1, INPUT_HANDLER_NONE, ntask_output_handler);

	nt1->next_nt(nt1, nt2);

	np->contain(np, 2, nt1, nt2);

	//nt1->set_affinity(nt1, MASK(0));
	//nt2->set_affinity(nt2, MASK(1));
	//nt1->set_partition(nt1, 0, 0);
	//nt2->set_partition(nt2, 1, 1);
	//nt->set_priority(nt, 10);
	
	printf("> prediction starts\n");

	// run input_handler+prediction is made
	np->run(np);

	printf("> prediction ends -> took %f seconds\n", np->get_time(np));
	printf("nt1 took %f seconds\n", nt1->get_time(nt1));
	printf("nt2 took %f seconds\n", nt2->get_time(nt2));

	// save image or show image on window
#ifdef OPENCV
        make_window("predictions", 512, 512, 0);

	int output_offset, output_size;
	nt2->get_data_output(nt2, &output_offset, &output_size);

	Image *im_out = (Image *)(nt2->get_memory_pointer(nt2)+output_offset);
        show_image(*im_out, "predictions", 0);
#else
        save_image(*im_out, "predictions"); // save on file
#endif
	// ntask destroy
	np->exit(np);
}
