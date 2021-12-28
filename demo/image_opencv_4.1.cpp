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

#ifdef OPENCV

#include <stdio.h>
#include <stdlib.h>
#include "opencv2/opencv.hpp"
#include "image.h"

using namespace cv;

extern "C" {

//IplImage *image_to_ipl(Image im)
Mat image_to_mat(Image im)
{
    assert(im.c == 3 || im.c == 1);
    int x,y,c;
    //IplImage *disp = cvCreateImage(cvSize(im.w,im.h), IPL_DEPTH_8U, im.c);
    //int step = disp->widthStep;
    Image copy = image_copy(im);
    constrain_image(copy);
    if(im.c == 3) rgbgr_image(copy);
    Mat m(im.h, im.w, CV_MAKETYPE(CV_8U, im.c));
    for(y = 0; y < im.h; ++y){
        for(x = 0; x < im.w; ++x){
            for(c= 0; c < im.c; ++c){
                //float val = im.data[c*im.h*im.w + y*im.w + x];
                //disp->imageData[y*step + x*im.c + c] = (unsigned char)(val*255);
		float val = copy.data[c*im.h*im.w + y*im.w + x];
                m.data[y*im.w*im.c + x*im.c + c] = (unsigned char)(val*255);
            }
        }
    }
    //return disp;
    image_free(copy);
    return m;
}

//Image ipl_to_image(IplImage* src)
Image mat_to_image(Mat m)
{
    //int h = src->height;
    //int w = src->width;
    //int c = src->nChannels;
    int h = m.rows;
    int w = m.cols;
    int c = m.channels();
    Image im = make_image(w, h, c);
    //unsigned char *data = (unsigned char *)src->imageData;
    //int step = src->widthStep;
    unsigned char *data = (unsigned char*)m.data;
    int step = m.step;
    int i, j, k;

    for(i = 0; i < h; ++i){
        for(k= 0; k < c; ++k){
            for(j = 0; j < w; ++j){
                im.data[k*w*h + i*w + j] = data[i*step + j*c + k]/255.;
            }
        }
    }
/*
    //return im;
//}

Mat image_to_mat(Image im)
{
    Image copy = image_copy(im);
    constrain_image(copy);
    if(im.c == 3) rgbgr_image(copy);

    IplImage *ipl = image_to_ipl(copy);
    Mat m = cvarrToMat(ipl, true);
    cvReleaseImage(&ipl);
    image_free(copy);
    return m;
}

Image mat_to_image(Mat m)
{
    IplImage ipl = m;
    Image im = ipl_to_image(&ipl);
*/
    rgbgr_image(im);
    return im;
}

void *open_video_stream(const char *f, int c, int w, int h, int fps)
{
    VideoCapture *cap;
    if(f) cap = new VideoCapture(f);
    else cap = new VideoCapture(c);
    if(!cap->isOpened()) return 0;
    //if(w) cap->set(CV_CAP_PROP_FRAME_WIDTH, w);
    //if(h) cap->set(CV_CAP_PROP_FRAME_HEIGHT, w);
    //if(fps) cap->set(CV_CAP_PROP_FPS, w);
    if(w) cap->set(CAP_PROP_FRAME_WIDTH, w);
    if(h) cap->set(CAP_PROP_FRAME_HEIGHT, w);
    if(fps) cap->set(CAP_PROP_FPS, w);
    return (void *) cap;
}

Image get_image_from_stream(void *p)
{
    VideoCapture *cap = (VideoCapture *)p;
    Mat m;
    *cap >> m;
    if(m.empty()) return image_make_empty(0,0,0);
    return mat_to_image(m);
}

Image load_image_cv(char *filename, int channels)
{
    int flag = -1;
    if (channels == 0) flag = -1;
    else if (channels == 1) flag = 0;
    else if (channels == 3) flag = 1;
    else {
        fprintf(stderr, "OpenCV can't force load with %d channels\n", channels);
    }
    Mat m;
    m = imread(filename, flag);
    if(!m.data){
	int ret;
        fprintf(stderr, "Cannot load image \"%s\"\n", filename);
        char buff[256];
        sprintf(buff, "echo %s >> bad.list", filename);
        ret = system(buff);
	if (ret<=0)
		fprintf(stderr, "Cannot echo filename\n");

        return make_image(10,10,3);
        //exit(0);
    }
    Image im = mat_to_image(m);
    return im;
}

int show_image_cv(Image im, const char* name, int ms)
{
    Mat m = image_to_mat(im);
    imshow(name, m);
    int c = waitKey(ms);
    if (c != -1) c = c%256;
    return c;
}

void make_window(char *name, int w, int h, int fullscreen)
{
    namedWindow(name, WINDOW_NORMAL); 
    if (fullscreen) {
        //setWindowProperty(name, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	setWindowProperty(name, WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
    } else {
        resizeWindow(name, w, h);
        if(strcmp(name, "Demo") == 0) moveWindow(name, 0, 0);
    }
}

}

#endif
