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

#ifndef BOX_H
#define BOX_H

typedef struct
{
    float x, y, w, h;
} Box;

typedef struct detection
{
    Box bbox;
    int classes;
    float *prob;
    float *mask;
    float objectness;
    int sort_class;
} Detection;

//Detection *network_get_boxes(Network *net, int w, int h, float thresh, float hier, int *map, int relative, int *num);
void do_nms_sort(Detection *dets, int total, int classes, float thresh);
void do_nms_obj(Detection *dets, int total, int classes, float thresh);
#endif
