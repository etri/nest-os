#ifndef NN_OBJECT_LIST_H
#define NN_OBJECT_LIST_H
#include "nn_object.h"
#include "support_model.h"

extern NnObject nn_object_10;
extern NnObject nn_object_11;

static NnObjectList obj_list[] = 
{
	{DARKNET_YOLOV3_TINY, &nn_object_10},
	{DARKNET_YOLOV3, &nn_object_11}
};
#endif
