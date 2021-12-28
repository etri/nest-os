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
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#include "cwd.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

float colors[6][3] = { {1,0,1}, {0,0,1},{0,1,1},{0,1,0},{1,1,0},{1,0,0} };

float get_color(int c, int x, int max)
{
    float ratio = ((float)x/max)*5;
    int i = floor(ratio);
    int j = ceil(ratio);
    ratio -= i;
    float r = (1-ratio) * colors[i][c] + ratio*colors[j][c];
    //printf("%f\n", r);
    return r;
}

Image *image_create(int c, int h, int w)
{
	Image *im;

	im = malloc(sizeof(Image));
	im->c = c;
	im->h = h;
	im->w = w;
	im->data = malloc(sizeof(float)*c*h*w);

	return im;
}

void image_free(Image m)
{
    if(m.data)
    {
        free(m.data);
    }
}

Image image_make_empty(int w, int h, int c)
{
    Image out;
    out.data = 0;
    out.h = h;
    out.w = w;
    out.c = c;

    return out;
}

Image image_copy(Image p)
{
    Image copy = p;
    copy.data = calloc(p.h*p.w*p.c, sizeof(float));
    memcpy(copy.data, p.data, p.h*p.w*p.c*sizeof(float));

    return copy;
}

Image make_image(int w, int h, int c)
{
    Image out = image_make_empty(w,h,c);
    out.data = calloc(h*w*c, sizeof(float));

    return out;
}

void constrain_image(Image im)
{
    int i;
    for(i = 0; i < im.w*im.h*im.c; ++i){
        if(im.data[i] < 0) im.data[i] = 0;
        if(im.data[i] > 1) im.data[i] = 1;
    }
}

void rgbgr_image(Image im)
{
    int i;
    for(i = 0; i < im.w*im.h; ++i){
        float swap = im.data[i];
        im.data[i] = im.data[i+im.w*im.h*2];
        im.data[i+im.w*im.h*2] = swap;
    }
}

void draw_box(Image a, int x1, int y1, int x2, int y2, float r, float g, float b)
{
    //normalize_image(a);
    int i;
    if(x1 < 0) x1 = 0;
    if(x1 >= a.w) x1 = a.w-1;
    if(x2 < 0) x2 = 0;
    if(x2 >= a.w) x2 = a.w-1;

    if(y1 < 0) y1 = 0;
    if(y1 >= a.h) y1 = a.h-1;
    if(y2 < 0) y2 = 0;
    if(y2 >= a.h) y2 = a.h-1;

    for(i = x1; i <= x2; ++i){
        a.data[i + y1*a.w + 0*a.w*a.h] = r;
        a.data[i + y2*a.w + 0*a.w*a.h] = r;

        a.data[i + y1*a.w + 1*a.w*a.h] = g;
        a.data[i + y2*a.w + 1*a.w*a.h] = g;

        a.data[i + y1*a.w + 2*a.w*a.h] = b;
        a.data[i + y2*a.w + 2*a.w*a.h] = b;
    }
    for(i = y1; i <= y2; ++i){
        a.data[x1 + i*a.w + 0*a.w*a.h] = r;
        a.data[x2 + i*a.w + 0*a.w*a.h] = r;

        a.data[x1 + i*a.w + 1*a.w*a.h] = g;
        a.data[x2 + i*a.w + 1*a.w*a.h] = g;

        a.data[x1 + i*a.w + 2*a.w*a.h] = b;
        a.data[x2 + i*a.w + 2*a.w*a.h] = b;
    }
}

void draw_box_width(Image a, int x1, int y1, int x2, int y2, int w, float r, float g, float b)
{
    int i;

    for(i = 0; i < w; ++i)
    {
        draw_box(a, x1+i, y1+i, x2-i, y2-i, r, g, b);
    }
}

static float get_pixel(Image m, int x, int y, int c)
{
    assert(x < m.w && y < m.h && c < m.c);
    return m.data[c*m.h*m.w + y*m.w + x];
}

static float get_pixel_extend(Image m, int x, int y, int c)
{
    if(x < 0 || x >= m.w || y < 0 || y >= m.h) 
	    return 0;
    /*
    if(x < 0) x = 0;
    if(x >= m.w) x = m.w-1;
    if(y < 0) y = 0;
    if(y >= m.h) y = m.h-1;
    */
    if(c < 0 || c >= m.c) 
	    return 0;

    return get_pixel(m, x, y, c);
}

static void set_pixel(Image m, int x, int y, int c, float val)
{
    if (x < 0 || y < 0 || c < 0 || x >= m.w || y >= m.h || c >= m.c) return;
    assert(x < m.w && y < m.h && c < m.c);
    m.data[c*m.h*m.w + y*m.w + x] = val;
}

static void add_pixel(Image m, int x, int y, int c, float val)
{
    assert(x < m.w && y < m.h && c < m.c);
    m.data[c*m.h*m.w + y*m.w + x] += val;
}

void embed_image(Image source, Image dest, int dx, int dy)
{
    int x,y,k;
    for(k = 0; k < source.c; ++k){
        for(y = 0; y < source.h; ++y){
            for(x = 0; x < source.w; ++x){
                float val = get_pixel(source, x,y,k);
                set_pixel(dest, dx+x, dy+y, k, val);
            }
        }
    }
}

void composite_image(Image source, Image dest, int dx, int dy)
{
    int x,y,k;
    for(k = 0; k < source.c; ++k){
        for(y = 0; y < source.h; ++y){
            for(x = 0; x < source.w; ++x){
                float val = get_pixel(source, x, y, k);
                float val2 = get_pixel_extend(dest, dx+x, dy+y, k);
                set_pixel(dest, dx+x, dy+y, k, val * val2);
            }
        }
    }
}

static void fill_cpu(int N, float ALPHA, float *X, int INCX)
{
    int i;
    for(i = 0; i < N; ++i) X[i*INCX] = ALPHA;
}

Image tile_images(Image a, Image b, int dx)
{
    if (a.w == 0) 
	    return image_copy(b);

    Image c = make_image(a.w + b.w + dx, (a.h > b.h) ? a.h : b.h, (a.c > b.c) ? a.c : b.c);
    fill_cpu(c.w*c.h*c.c, 1, c.data, 1);

    embed_image(a, c, 0, 0);
    composite_image(b, c, a.w + dx, 0);

    return c;
}

Image border_image(Image a, int border)
{
    Image b = make_image(a.w + 2*border, a.h + 2*border, a.c);
    int x,y,k;

    for(k = 0; k < b.c; ++k){
        for(y = 0; y < b.h; ++y){
            for(x = 0; x < b.w; ++x){
                float val = get_pixel_extend(a, x - border, y - border, k);
                if(x - border < 0 || x - border >= a.w || y - border < 0 || y - border >= a.h) val = 1;
                set_pixel(b, x, y, k, val);
            }
        }
    }
    return b;
}

void fill_image(Image m, float s)
{
    int i;
    for(i = 0; i < m.h*m.w*m.c; ++i) m.data[i] = s;
}

Image get_label(Image **characters, char *string, int size)
{
    size = size/10;
    if(size > 7) size = 7;
    Image label = image_make_empty(0,0,0);

    while(*string)
    {
        Image l = characters[size][(int)*string];
        Image n = tile_images(label, l, -size - 1 + (size+1)/2);

        image_free(label);
        label = n;
        ++string;
    }

    Image b = border_image(label, label.h*.25);

    image_free(label);

    return b;
}

void draw_label(Image a, int r, int c, Image label, const float *rgb)
{
    int w = label.w;
    int h = label.h;
    if (r - h >= 0) r = r - h;

    int i, j, k;
    for(j = 0; j < h && j + r < a.h; ++j){
        for(i = 0; i < w && i + c < a.w; ++i){
            for(k = 0; k < label.c; ++k){
                float val = get_pixel(label, i, j, k);
                set_pixel(a, i+c, j+r, k, rgb[k] * val);
            }
        }
    }
}

void draw_detections(Image im, Detection *dets, int num, float thresh, char **names, Image **alphabet, int classes)
{
    int i,j;

    for(i = 0; i < num; ++i){
        char labelstr[4096] = {0};
        int class = -1;
        for(j = 0; j < classes; ++j){
            if (dets[i].prob[j] > thresh){
                if (class < 0) {
                    strcat(labelstr, names[j]);
                    class = j;
                } else {
                    strcat(labelstr, ", ");
                    strcat(labelstr, names[j]);
                }
                printf("%s: %.0f%%\n", names[j], dets[i].prob[j]*100);
            }
        }
        if(class >= 0){
            int width = im.h * .006;

            /*
               if(0){
               width = pow(prob, 1./2.)*10+1;
               alphabet = 0;
               }
             */

            //printf("%d %s: %.0f%%\n", i, names[class], prob*100);
            int offset = class*123457 % classes;
            float red = get_color(2,offset,classes);
            float green = get_color(1,offset,classes);
            float blue = get_color(0,offset,classes);
            float rgb[3];

            //width = prob*20+2;

            rgb[0] = red;
            rgb[1] = green;
            rgb[2] = blue;
            Box b = dets[i].bbox;
            //printf("%f %f %f %f\n", b.x, b.y, b.w, b.h);

			// calculate the exact coordinate points from width, height
            int left  = (b.x-b.w/2.)*im.w;
            int right = (b.x+b.w/2.)*im.w;
            int top   = (b.y-b.h/2.)*im.h;
            int bot   = (b.y+b.h/2.)*im.h;

			// processing minus coordinate values
            if(left < 0) left = 0;
            if(right > im.w-1) right = im.w-1;
            if(top < 0) top = 0;
            if(bot > im.h-1) bot = im.h-1;

            draw_box_width(im, left, top, right, bot, width, red, green, blue); // 박스를 그린다.
            if (alphabet) {
                Image label = get_label(alphabet, labelstr, (im.h*.03)); // 라벨을 그린다.
                draw_label(im, top + width, left, label, rgb);
                image_free(label);
            }
#if 0
            if (dets[i].mask){ // 0
                Image mask = float_to_image(14, 14, 1, dets[i].mask);
                Image resized_mask = resize_image(mask, b.w*im.w, b.h*im.h);
                Image tmask = threshold_image(resized_mask, .5);
                embed_image(tmask, im, left, top);
                image_free(mask);
                image_free(resized_mask);
                image_free(tmask);
            }
#endif
        }
    }
}

void free_detections(Detection *dets, int n)
{
    int i;

    for(i = 0; i < n; ++i)
    {
        free(dets[i].prob);
        if(dets[i].mask) free(dets[i].mask);
    }
    free(dets);
}

Image resize_image(Image im, int w, int h)
{
    Image resized = make_image(w, h, im.c);
    Image part = make_image(w, im.h, im.c);
    int r, c, k;
    float w_scale = (float)(im.w - 1) / (w - 1);
    float h_scale = (float)(im.h - 1) / (h - 1);
    for(k = 0; k < im.c; ++k){
        for(r = 0; r < im.h; ++r){
            for(c = 0; c < w; ++c){
                float val = 0;
                if(c == w-1 || im.w == 1){
                    val = get_pixel(im, im.w-1, r, k);
                } else {
                    float sx = c*w_scale;
                    int ix = (int) sx;
                    float dx = sx - ix;
                    val = (1 - dx) * get_pixel(im, ix, r, k) + dx * get_pixel(im, ix+1, r, k);
                }
                set_pixel(part, c, r, k, val);
            }
        }
    }
    for(k = 0; k < im.c; ++k){
        for(r = 0; r < h; ++r){
            float sy = r*h_scale;
            int iy = (int) sy;
            float dy = sy - iy;
            for(c = 0; c < w; ++c){
                float val = (1-dy) * get_pixel(part, c, iy, k);
                set_pixel(resized, c, r, k, val);
            }
            if(r == h-1 || im.h == 1) continue;
            for(c = 0; c < w; ++c){
                float val = dy * get_pixel(part, c, iy+1, k);
                add_pixel(resized, c, r, k, val);
            }
        }
    }

    image_free(part);
    return resized;
}

Image load_image_stb(char *filename, int channels)
{
    int w, h, c;
    unsigned char *data = stbi_load(filename, &w, &h, &c, channels);

    if (!data) 
    {
        fprintf(stderr, "Cannot load image \"%s\"\nSTB Reason: %s\n", filename, stbi_failure_reason());
        exit(0);
    }

    if(channels) 
	    c = channels;

    int i,j,k;
    Image im = make_image(w, h, c);

    for(k = 0; k < c; ++k)
    {
        for(j = 0; j < h; ++j)
	{
            for(i = 0; i < w; ++i)
	    {
                int dst_index = i + w*j + w*h*k;
                int src_index = k + c*i + c*w*j;
                im.data[dst_index] = (float)data[src_index]/255.;
            }
        }
    }

    free(data);

    return im;
}

Image load_image(char *filename, int w, int h, int c)
{
#ifdef OPENCV
    Image out = load_image_cv(filename, c);
#else
    Image out = load_image_stb(filename, c);
#endif

    if((h && w) && (h != out.h || w != out.w)){ // h=0, w=0, ignored
        Image resized = resize_image(out, w, h);
        image_free(out);
        out = resized;
    }
    return out;
}

Image load_image_color(char *filename, int w, int h)
{
    return load_image(filename, w, h, 3);
}

Image **load_alphabet(void)
{
    int i, j;
    const int nsize = 8;
    Image **alphabets = calloc(nsize, sizeof(Image));
    for(j = 0; j < nsize; ++j){ /* alphabet size */
        alphabets[j] = calloc(128, sizeof(Image));
        for(i = 32; i < 127; ++i){ /* ascii code number of the alphabet */
            char buff[256];
            sprintf(buff, WORK_DIR()"data/labels/%d_%d.png", i, j);
            alphabets[j][i] = load_image_color(buff, 0, 0);
        }
    }
    return alphabets;
}

void save_image_options(Image im, const char *name, IMTYPE f, int quality)
{
    char buff[256];

    //sprintf(buff, "%s (%d)", name, windows);
    if(f == PNG)       sprintf(buff, "%s.png", name);
    else if (f == BMP) sprintf(buff, "%s.bmp", name);
    else if (f == TGA) sprintf(buff, "%s.tga", name);
    else if (f == JPG) sprintf(buff, "%s.jpg", name);
    else               sprintf(buff, "%s.png", name);

    unsigned char *data = calloc(im.w*im.h*im.c, sizeof(char));
    int i,k;

    for(k = 0; k < im.c; ++k){
        for(i = 0; i < im.w*im.h; ++i){
            data[i*im.c+k] = (unsigned char) (255*im.data[i + k*im.w*im.h]);
        }
    }

    int success = 0;

    if(f == PNG)       success = stbi_write_png(buff, im.w, im.h, im.c, data, im.w*im.c);
    else if (f == BMP) success = stbi_write_bmp(buff, im.w, im.h, im.c, data);
    else if (f == TGA) success = stbi_write_tga(buff, im.w, im.h, im.c, data);
    else if (f == JPG) success = stbi_write_jpg(buff, im.w, im.h, im.c, data, quality);
    free(data);
    if(!success) fprintf(stderr, "Failed to write image %s\n", buff);
}

void save_image(Image im, const char *name)
{
    save_image_options(im, name, JPG, 80);
}

int show_image(Image p, const char *name, int ms)
{
#ifdef OPENCV
    int c = show_image_cv(p, name, ms);
    return c;
#else
    fprintf(stderr, "Not compiled with OpenCV, saving to %s.png instead\n", name);
    save_image(p, name);
    return -1;
#endif
}

Image letterbox_image(Image im, int w, int h)
{
    int new_w = im.w;
    int new_h = im.h;
    if (((float)w/im.w) < ((float)h/im.h)) {
        new_w = w;
        new_h = (im.h * w)/im.w;
    } else {
        new_h = h;
        new_w = (im.w * h)/im.h;
    }
    Image resized = resize_image(im, new_w, new_h); // 3 x 312 x 416
    Image boxed = make_image(w, h, im.c); // 3 x 416 x 416
    fill_image(boxed, .5);
    //int i;
    //for(i = 0; i < boxed.w*boxed.h*boxed.c; ++i) boxed.data[i] = 0;
    embed_image(resized, boxed, (w-new_w)/2, (h-new_h)/2);
    image_free(resized);
    return boxed;
}

void letterbox_image_into(Image im, int w, int h, Image boxed)
{
    int new_w = im.w;
    int new_h = im.h;
    if (((float)w/im.w) < ((float)h/im.h)) 
    {
        new_w = w;
        new_h = (im.h * w)/im.w;
    } else 
    {
        new_h = h;
        new_w = (im.w * h)/im.h;
    }
    Image resized = resize_image(im, new_w, new_h);
    embed_image(resized, boxed, (w-new_w)/2, (h-new_h)/2); 
    image_free(resized);
}
