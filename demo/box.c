/*********************************************************************
	DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                   Version 2, December 2004

Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

Everyone is permitted to copy and distribute verbatim or modified
copies of this license document, and changing it is allowed as long
as the name is changed.

           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
  TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

 0. You just DO WHAT THE FUCK YOU WANT TO.
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "box.h"

float overlap(float x1, float w1, float x2, float w2)
{
    float l1 = x1 - w1/2;
    float l2 = x2 - w2/2;
    float left = l1 > l2 ? l1 : l2;
    float r1 = x1 + w1/2;
    float r2 = x2 + w2/2;
    float right = r1 < r2 ? r1 : r2;

    return right - left;
}

float box_intersection(Box a, Box b)
{
    float w = overlap(a.x, a.w, b.x, b.w);
    float h = overlap(a.y, a.h, b.y, b.h);

    if (w < 0 || h < 0) 
	    return 0;

    float area = w*h;

    return area;
}

float box_union(Box a, Box b)
{
    float i = box_intersection(a, b);
    float u = a.w*a.h + b.w*b.h - i;

    return u;
}

float box_iou(Box a, Box b)
{
    return box_intersection(a, b)/box_union(a, b);
}

int nms_comparator(const void *pa, const void *pb)
{
    Detection a = *(Detection *)pa;
    Detection b = *(Detection *)pb;

    float diff = 0;

    if(b.sort_class >= 0)
    {
        diff = a.prob[b.sort_class] - b.prob[b.sort_class];
    }
    else 
    {
        diff = a.objectness - b.objectness;
    }

    if(diff < 0) 
	    return 1;
    else if (diff > 0) 
	    return -1;

    return 0;
}

void do_nms_sort(Detection *dets, int total, int classes, float thresh)
{
    int i, j, k;

    k = total-1;

    for(i = 0; i <= k; ++i)
    {
        if(dets[i].objectness == 0)
	{
            Detection swap = dets[i];
            dets[i] = dets[k];
            dets[k] = swap;
            --k;
            --i;
        }
    }
    total = k+1;

    for(k = 0; k < classes; ++k)
    { // for all classes, do sort probabilities for each class
        for(i = 0; i < total; ++i)
	{
            dets[i].sort_class = k;
        }
        qsort(dets, total, sizeof(Detection), nms_comparator);
	//dets_fprint(dets, total, classes, "./dets.ccc");
        for(i = 0; i < total; ++i)
	{
            if(dets[i].prob[k] == 0) 
		    continue;

            Box a = dets[i].bbox;
            for(j = i+1; j < total; ++j)
	    {
                Box b = dets[j].bbox;
                if (box_iou(a, b) > thresh)
		{ // duplicated
                    dets[j].prob[k] = 0; // it is removed
                }
            }
        }
    }
}

void do_nms_obj(Detection *dets, int total, int classes, float thresh)
{
    int i, j, k;
    k = total-1;
    for(i = 0; i <= k; ++i){
        if(dets[i].objectness == 0){
            Detection swap = dets[i];
            dets[i] = dets[k];
            dets[k] = swap;
            --k;
            --i;
        }
    }
    total = k+1;

    for(i = 0; i < total; ++i){
        dets[i].sort_class = -1;
    }

    qsort(dets, total, sizeof(Detection), nms_comparator);
    for(i = 0; i < total; ++i){
        if(dets[i].objectness == 0) continue;
        Box a = dets[i].bbox;
        for(j = i+1; j < total; ++j){
            if(dets[j].objectness == 0) continue;
            Box b = dets[j].bbox;
            if (box_iou(a, b) > thresh){
                dets[j].objectness = 0;
                for(k = 0; k < classes; ++k){
                    dets[j].prob[k] = 0;
                }
            }
        }
    }
}

#if 0
int num_detections(Network *net, float thresh)
{
    int s = 0;

    for (Layer *l=net->head; l!=NULL; l=l->next)
    {
        if (l->type == REGION)
        {
                Matrix *out = l->Lout;

                s += out->dim[2] * out->dim[0]; // 5*169
        }
    }

    return s;
}

Detection *network_make_boxes(Network *net, float thresh, int *num)
{
    RegionLayer *l = (RegionLayer *)net->tail; // the last layer
    Detection *dets;
    int i;
    int nboxes = num_detections(net, thresh); // nboxes = 845 = 13x13x5

    if (num)
    {
            *num = nboxes; // nboxes = 845
    }

    dets = calloc(nboxes, sizeof(Detection));

    for(i = 0; i < nboxes; ++i)
    {
        dets[i].prob = calloc(l->classes, sizeof(float)); // l.classes = 80

        if(l->coords > 4)
        { // l.coords = 4
            dets[i].mask = calloc(l->coords-4, sizeof(float));
        }
    }

    return dets;
}

Box get_region_box(Matrix *m, float *biases, int n, int k, int i, int j, int w, int h)
{
    Box b;

    b.x = (i + Data3D(m, n, 0, k)) / w; // stride = 169, w = 13
    b.y = (j + Data3D(m, n, 1, k)) / h; // stride = 169, h = 13
    b.w = exp(Data3D(m, n, 2, k)) * biases[2*n]   / w;
    b.h = exp(Data3D(m, n, 3, k)) * biases[2*n+1] / h;

    return b;
}

void correct_region_boxes(Detection *dets, int n, int w, int h, int netw, int neth, int relative)
{
    int i;
    int new_w=0;
    int new_h=0;

    if (((float)netw/w) < ((float)neth/h))
    { // execute this
        new_w = netw;
        new_h = (h * netw)/w;
    }
    else
    {
        new_h = neth;
        new_w = (w * neth)/h;
    }

    for (i = 0; i < n; ++i)
    { // n = 845, w=768, h=576, netw=416, neth=416, relative = 1
        Box b = dets[i].bbox;

        b.x =  (b.x - (netw - new_w)/2./netw) / ((float)new_w/netw);
        b.y =  (b.y - (neth - new_h)/2./neth) / ((float)new_h/neth);
        b.w *= (float)netw/new_w;
        b.h *= (float)neth/new_h;

        if(!relative)
        {
            b.x *= w;
            b.w *= w;
            b.y *= h;
            b.h *= h;
        }
        dets[i].bbox = b;
    }
}

void get_region_detections(Layer *l_in, int w, int h, int netw, int neth, float thresh, int *map, float tree_thresh, int relative, Detection *dets)
{
    int i,j,n;
    RegionLayer *l = (RegionLayer *)l_in;
    Matrix *m = l->Lout;

    for (i = 0; i < l->w * l->h; ++i)
    { // for each 13x13 grids, row = 169
        int row = i / l->w;
        int col = i % l->w;

        for(n = 0; n < l->detectors; ++n)
        { // l.n = 5 boxes in each grid, col = 5
            int index = n*l->w*l->h + i; // 5x13x13, 0, 169, ...

            for(j = 0; j < l->classes; ++j)
            {
                dets[index].prob[j] = 0;
            }

	    float scale = Data3D(m, n, 4, i); // l.background = 0

	    dets[index].bbox = get_region_box(m, l->b->data, n, i, col, row, l->w, l->h);
            dets[index].objectness = scale > thresh ? scale : 0;

	    if(dets[index].objectness)
            {
                for(j = 0; j < l->classes; ++j)
                {
                    float value = Data3D(m, n, l->coords + 1 + j, i);
                    float prob = scale*value;

                    dets[index].prob[j] = (prob > thresh) ? prob : 0;
                }
            }
        }
    }
    correct_region_boxes(dets, l->w*l->h*l->detectors, w, h, netw, neth, relative);
}

void network_fill_boxes(Network *net, int w, int h, float thresh, float hier, int *map, int relative, Detection *dets)
{
    for (Layer *l=net->head; l!=NULL; l=l->next)
    {
        if(l->type == REGION)
        {
            get_region_detections(l, w, h, net->xm->dim[1], net->xm->dim[0], thresh, map, hier, relative, dets);
	}
    }
}

Detection *network_get_boxes(Network *net, int w, int h, float thresh, float hier, int *map, int relative, int *num)
{
    Detection *dets;

    dets = network_make_boxes(net, thresh, num); // dets[845] = dets[13x13x5]

    network_fill_boxes(net, w, h, thresh, hier, map, relative, dets);

    return dets;
}
#endif

