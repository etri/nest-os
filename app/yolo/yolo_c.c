/////////// File input and Loader is included ///////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ntask.h"
#include "util_time.h"
#include "demo/image.h"
#include "loader.h"
#include "config.h" // model name

static char *image_path;

static void ntask_input_handler(Ntask *nt)
{
	nt->set_file_input(nt, image_path);
}

int output_offset, output_size;

static void ntask_output_handler(Ntask *nt)
{
	nt->get_data_output(nt, &output_offset, &output_size);
        printf("output data : offset = %d size = %d\n", output_offset, output_size);

	// get the output data, and adjust the image data pointer if necessary
        Image *im_out = (Image *)(nt->get_memory_pointer(nt)+output_offset);
	im_out->data = (float *)(nt->get_memory_pointer(nt)+output_offset+32);
}

int main(int argc, char *argv[])
{
	/* Load Kernel */
	Loader *loader= loader_create();

	loader->set_file(loader, "yolov2.weights");
	loader->load(loader, MODEL_NAME, 0);

	image_path = argc==2? argv[1]:"data/dog.jpg";
	printf("argc = %d argv[1]=%s\n", argc, argv[1]);

	// ntask create and do setting if necessary (affinity or priority)
	Ntask *nt = ntask_create("nt", MODEL_NAME, ntask_input_handler, ntask_output_handler);
	//nt->set_affinity(nt, MASK(2));
	//nt->set_priority(nt, 10);
	
	printf("> prediction starts\n");
	double time=what_time_is_it_now();

	// run input_handler+prediction is made
	nt->run(nt);

	printf("> prediction ends -> took %f seconds\n", what_time_is_it_now() - time);

	// save image or show image on window
#ifdef OPENCV
        make_window("predictions", 512, 512, 0);
	Image *im_out = (Image *)(nt->get_memory_pointer(nt)+output_offset);
        show_image(*im_out, "predictions", 0);
#else
        save_image(*im_out, "predictions"); // save on file
#endif
	// ntask destroy
	nt->exit(nt);

	loader->unload(loader, MODEL_NAME, 0);
}
