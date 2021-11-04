#ifndef _LIBYOLOTALK_H_
#define _LIBYOLOTALK_H_

#include "yolo.h"
#include "safe_queue.hpp"
#include <thread>
#include <opencv2/opencv.hpp>

class YoloDevice
{
private:
    Yolo *yolo = NULL;
    char *cfg;
    char *weights;
    char *name_list;
    char *url;
    float thresh = 0.5;
    char *output_folder = NULL;
    int max_video_queue_size;
    std::vector<yolotalk::Point> *vertices = NULL;
    yolotalk::Point* vertices_data = NULL; 
    SafeQueue<Mat*> *videoQueue;
    bool show_msg;

    std::thread *videoThread;
    std::thread *predictionThread;

    bool running = false;
    bool force_stop = false;

    float modelFps = 0;
    float videoFps = 0;
    float overAllFps = 0;

    void (*predictionListener)(int frame_id, Mat *mat, BoundingBox **bboxes, int bbox_len, char* file_path) = NULL;

    std::vector<yolotalk::Point> *parseVertices(float *vertices, int vertices_size);
    void warmUpModel();
    void videoCaptureLoop();
    void predictionLoop();
    void runCallback(int frame_id, Mat *mat, std::vector<BoundingBox*> *boxes);
    void print_msg(const char *__restrict __fmt, ...);
    void print_cmd_msg(char *cmd);
    std::string get_full_output_folder_path();

public:
    YoloDevice(char *cfg, char *weights, char *name_list, char *url, float thresh, float *vertices, int vertices_size, char *output_folder, int max_video_queue_size, bool show_msg=false);
    void start();
    void setPolygon(float *vertices, int vertices_size);
    void setPolygon(std::vector<yolotalk::Point> *vertex_points);
    void setPredictionListener(void (*func)(int frame_id, Mat *mat, BoundingBox **bboxes, int bbox_len, char* file_path));
    void join();
    void stop(bool force=false);    
    float getVideoFps();
    float getModelFps();
    float getFps();
    void getColors(int class_id, float *return_r, float* return_g, float* return_b);
    std::vector<yolotalk::Point> *getPolygon();
    yolotalk::Point *getPolygonData();
};

#endif