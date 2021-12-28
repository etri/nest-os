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

#ifndef IMAGE_H
#define IMAGE_H
#include "box.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
    PNG, BMP, TGA, JPG
} IMTYPE;

typedef struct 
{
    int w;
    int h;
    int c;
    float *data;
} Image;

Image *image_create(int c, int h, int w);
Image **load_alphabet(void);
void draw_detections(Image im, Detection *dets, int num, float thresh, char **names, Image **alphabet, int classes);
void free_detections(Detection *dets, int n);
void save_image(Image im, const char *name);
Image load_image_color(char *filename, int w, int h);
Image letterbox_image(Image im, int w, int h);
void letterbox_image_into(Image im, int w, int h, Image boxed);
Image make_image(int w, int h, int c);
int show_image(Image p, const char *name, int ms);
void image_free(Image m);
float get_color(int c, int x, int max);
Image get_label(Image **characters, char *string, int size);
void draw_label(Image a, int r, int c, Image label, const float *rgb);
Image image_copy(Image p);
void constrain_image(Image im);
void rgbgr_image(Image im);
Image image_make_empty(int w, int h, int c);

#ifdef OPENCV
void *open_video_stream(const char *f, int c, int w, int h, int fps);
Image get_image_from_stream(void *p);
Image load_image_cv(char *filename, int channels);
int show_image_cv(Image im, const char* name, int ms);
void make_window(char *name, int w, int h, int fullscreen);
#endif

#ifdef __cplusplus
}
#endif

#endif
