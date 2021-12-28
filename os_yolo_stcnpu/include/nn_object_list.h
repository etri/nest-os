#ifndef NN_OBJECT_LIST_H
#define NN_OBJECT_LIST_H
#include "nn_object.h"
#include "support_model.h"

extern NnObject nn_object_1;
extern NnObject nn_object_11;
extern NnObject nn_object_12;
extern NnObject nn_object_129;
extern NnObject nn_object_130;
extern NnObject nn_object_133;

static NnObjectList obj_list[] = 
{
	{YOLOV2_NPU, &nn_object_1},
	{YOLOV2_NPU1, &nn_object_11},
	{YOLOV2_NPU2, &nn_object_12},
	{YOLOV2_CPU, &nn_object_129},
	{YOLOV2_TINY_CPU, &nn_object_130},
	{YOLOV2_TINY_CPU_PART, &nn_object_133}
};
#endif
