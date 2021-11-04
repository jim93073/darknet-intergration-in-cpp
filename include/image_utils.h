#ifndef _IMAGE_UTILS_H_
#define _IMAGE_UTILS_H_
#include "opencv2/opencv.hpp"
#include "darknet.h"

using namespace cv;

image ipl_to_image(IplImage *src);
image mat_to_image(Mat m);
void *open_video_stream(const char *f, int c, int w, int h, int fps);
image get_image_from_stream(void *p);
#endif
