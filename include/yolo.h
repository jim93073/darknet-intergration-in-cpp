#ifndef _YOLO_H_
#define _YOLO_H_
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <thread>
#include <box.h>

#include <yolotalk_utils.h>

#include <opencv2/opencv.hpp>

using namespace cv;

class BoundingBox {
public:
    BoundingBox(char*, int, float, box);
    int getXmin();
    int getXmax();
    int getYmin();
    int getYmax();
    int getClassId();
    char* getName();
    float getConfidence();
    box getBox();
private:
    int classId;
    char* name;
    float confidence;
    box boxIn;
};

class Yolo
{
public:
    Yolo(char *, char *, char *);
    Yolo(char *cfg, char *weights, char *name_list, std::vector<yolotalk::Point> *vertex_points);
    ~Yolo();
    std::vector<BoundingBox*> *detect_image(Mat *frame, float thresh=0.5, float nms=0.45);
    int getWidth();
    int getHeight();
    int getColorRed(int);
    int getColorGreen(int);
    int getColorBlue(int);
    void setPolygon(std::vector<yolotalk::Point> *vertex_points);
private:
    // detection_with_class *predict(Mat, int *, float, float);
    int names_size = 0;
    network net;
    layer l;
    int width, height;
    int classes;
    char **names;
    std::vector<yolotalk::Point> *vertex_points;
    void init(char *cfg, char *weights, char *name_list, std::vector<yolotalk::Point> *vertex_points);
};
#endif
