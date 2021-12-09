#include <libyolotalk.h>
#include "clibyolotalk.h"
#include <utils.h>
#include <yolotalk_utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <signal.h>
#include <mutex>
#include <chrono>


int global_ID = 0;
YoloDevice::YoloDevice(char *cfg, char *weights, char *name_list, char *url, float thresh, float *vertices, int vertices_size, char *output_folder, int max_video_queue_size, bool show_msg)
{
    this->cfg = cfg;
    this->weights = weights;
    this->name_list = name_list;
    this->url = url;
    this->thresh = thresh;
    this->output_folder = output_folder;
    this->setPolygon(vertices, vertices_size);
    this->videoQueue = new SafeQueue<Mat *>();
    this->max_video_queue_size = max_video_queue_size;
    this->show_msg = show_msg;  
    this->ID = global_ID;
    global_ID += 1;
}


void YoloDevice::start()
{
    // Init YOLO Model
    this->print_msg("%s", "Loading Model");
    this->print_cmd_msg(YOLOTALK_CMD_LOADING_MODEL);
    if (this->vertices == NULL)
    {
        this->yolo = new Yolo(cfg, weights, name_list);
    }
    else
    {
        this->yolo = new Yolo(cfg, weights, name_list, this->vertices);
    }
    this->print_msg("%s", "Model loaded");
    this->print_cmd_msg(YOLOTALK_CMD_LOADING_MODEL_FINISH);

    this->warmUpModel();

    this->running = true;         
    
    this->videoThread = new std::thread(&YoloDevice::videoCaptureLoop, this);
    this->predictionThread = new std::thread(&YoloDevice::predictionLoop, this);
}

void YoloDevice::setPolygon(float *vertices, int vertices_size)
{
    if (vertices_size == 0)
    {
        return;
    }

    if (this->vertices_data != NULL)
    {
        free(this->vertices_data);
    }

    std::vector<yolotalk::Point> *points = parseVertices(vertices, vertices_size);
    this->setPolygon(points);

    this->vertices_data = (yolotalk::Point*) calloc(points->size(), sizeof(yolotalk::Point));
    memcpy(this->vertices_data, &vertices[0], points->size()*sizeof(yolotalk::Point));
}

void YoloDevice::setPolygon(std::vector<yolotalk::Point> *vertex_points)
{
    if (this->vertices != NULL)
    {
        for (int i = 0; i > this->vertices->size(); i++)
        {
            free(&(*this->vertices)[i]);
        }
        delete this->vertices;
    }

    this->vertices = vertex_points;
    if (this->yolo != NULL)
    {
        this->yolo->setPolygon(vertex_points);
    }
}

void YoloDevice::setPredictionListener(void (*func)(int frame_id, Mat *mat, BoundingBox **bboxes, int bbox_len, char *file_path))
{
    this->predictionListener = func;
}

void YoloDevice::join()
{
    this->predictionThread->join();
    this->videoThread->join();
}

void YoloDevice::stop(bool force)
{
    this->running = false;
    this->force_stop = force;
}

std::vector<yolotalk::Point> *YoloDevice::parseVertices(float *vertices, int vertices_size)
{
    if (vertices_size % 2 != 0)
    {
        throw "\"vertices_size\" should be an even number and greater than 6.";
    }

    std::vector<yolotalk::Point> *vertex_points = new std::vector<yolotalk::Point>();
    for (int i = 0; i < vertices_size; i += 2)
    {
        yolotalk::Point *p = (yolotalk::Point *) malloc(sizeof(yolotalk::Point));
        p->x = vertices[i];
        p->y = vertices[i + 1];
        vertex_points->push_back(*p);
    }
    return vertex_points;
}

void YoloDevice::warmUpModel()
{
    if (this->yolo == NULL)
    {
        return;
    }

    this->print_msg("%s", "Warming up");
    this->print_cmd_msg(YOLOTALK_CMD_WARMING_UP);

    Mat *img_w = new Mat(this->yolo->getWidth(), this->yolo->getHeight(), CV_8UC3);
    std::vector<BoundingBox *> *result = this->yolo->detect_image(img_w);
    delete result;
    img_w->release();
    delete img_w;

    this->print_msg("%s", "Warming up finished");
    this->print_cmd_msg(YOLOTALK_CMD_WARMING_UP_FINISH);
}

void YoloDevice::videoCaptureLoop()
{
    cv::VideoCapture cap(url);

    

    while (this->running){
        if (!cap.isOpened()){
            this->print_msg("ID:[%d] Cannot open video: %s", this->ID, this->url);
            std::chrono::milliseconds timespan(10000);
            std::this_thread::sleep_for(timespan);
        //         system("/home/jim/restart_sf.sh");        
        }
        this->ret = cap.read(this->frame);
        if (!this->ret) {           
            this->print_msg("ID:[%d] Cannot read frame: %s", this->ID, this->url);
            std::chrono::milliseconds timespan(3000);
            std::this_thread::sleep_for(timespan);
//             system("/home/jim/restart_sf.sh");
        }
    }
    this->running = false;
    cap.release();
}

void YoloDevice::predictionLoop()
{
    this->print_msg("Start prediction.%s", "");
    this->print_cmd_msg(YOLOTALK_CMD_START_PREDICT);
    int frame_id = 0;
    int ctn = 1;
    double fps, prediction_start_time, sum_prediction_time, loop_start_time, loop_time;
    double lastTime = what_time_is_it_now();

    while (this->running)
    {
        if (!this->ret){            
            std::chrono::milliseconds timespan(3000);
            std::this_thread::sleep_for(timespan);
            continue;
        }
        
        Mat *to_push = new Mat(Size(this->frame.cols, this->frame.rows), CV_8UC(this->frame.channels()));
        this->frame.copyTo(*to_push);
        Mat *frame_ = to_push;
        
        loop_start_time = what_time_is_it_now();
        prediction_start_time = what_time_is_it_now();
        std::vector<BoundingBox *> *boxes = yolo->detect_image(frame_, thresh, 0.45);
        sum_prediction_time += what_time_is_it_now() - prediction_start_time;

        this->runCallback(frame_id, frame_, boxes);
        // std::thread th(&YoloDevice::runCallback, this, frame_id, frame_, boxes);
        // std::thread::id tid = th.get_id();

        frame_id++;
        ctn++;
        
        if (what_time_is_it_now() - lastTime >= 1)
        {            
            fps = 1 / (sum_prediction_time / ctn);
            this->modelFps = fps;
            if (this->show_msg)
            {
                this->print_msg("[%d] FPS: %.1f,  URL:%s", this->ID, this->modelFps, this->url);                
            }
            if (!this->running)
            {
                if (this->force_stop)
                {
                    break;
                }                
            }
            lastTime = what_time_is_it_now();
            ctn = 0;
            sum_prediction_time = 0;
        }
        loop_time = what_time_is_it_now() - loop_start_time;
        this->overAllFps = 1.0 / loop_time;        
    }
}

std::string YoloDevice::get_full_output_folder_path()
{
    std::string sub_dir = Date() + "/" + TIME() + "/";
    std::string path = std::string(this->output_folder) + sub_dir;
    return path;
}

void YoloDevice::runCallback(int frame_id, Mat *mat, std::vector<BoundingBox *> *boxes)
{
    // save image
    if (output_folder != NULL)
    {
        std::string out_path = this->get_full_output_folder_path();
        ensure_dir((char *)out_path.c_str());

        char filename[10];
        sprintf(filename, "%05i", frame_id);

        std::string file_path = out_path + std::string(filename) + std::string(".jpg");
        cv::imwrite(file_path, *mat);

        // callback
        this->predictionListener(frame_id, mat, &(*boxes)[0], boxes->size(), (char *)file_path.c_str());
    }
    else
    {
        // callback
        this->predictionListener(frame_id, mat, &(*boxes)[0], boxes->size(), NULL);
    }
    
    mat->release();        
    delete mat;

    for (int i = 0; i < boxes->size(); i++)
    {
        delete ((*boxes)[i]);
    }

    delete boxes;
}

void YoloDevice::print_msg(const char *__restrict __fmt, ...)
{
    if (this->show_msg)
    {
        fprintf(stderr, "%s ", YOLOTALK_PREFIX);
        va_list args;
        va_start(args, __fmt);
        vfprintf(stderr, __fmt, args);
        va_end(args);
        fprintf(stderr, "%s", "\n");
    }
}

void YoloDevice::print_cmd_msg(char *cmd)
{
    if (this->show_msg)
    {
        // print_cmd(cmd);
        fprintf(stderr, "%s %s\n", YOLOTALK_CMD_PREFIX, cmd);
    }
}

float YoloDevice::getVideoFps()
{
    return this->videoFps;
}

float YoloDevice::getModelFps()
{
    return this->modelFps;
}

float YoloDevice::getFps()
{
    return this->overAllFps;
}

void YoloDevice::getColors(int class_id, float *return_r, float *return_g, float *return_b)
{
    if (this->yolo == NULL)
    {
        return;
    }
    *return_r = this->yolo->getColorRed(class_id);
    *return_g = this->yolo->getColorGreen(class_id);
    *return_b = this->yolo->getColorBlue(class_id);
}

std::vector<yolotalk::Point> *YoloDevice::getPolygon()
{
    return this->vertices;
}

yolotalk::Point *YoloDevice::getPolygonData()
{
    return this->vertices_data;
}

int getMatLength(cv::Mat *mat)
{
    return mat->rows * mat->cols * mat->channels();
}

void getMatInfo(cv::Mat *mat, int *rows, int *cols, int *channels)
{
    *rows = mat->rows;
    *cols = mat->cols;
    *channels = mat->channels();
}

void matToArray(cv::Mat *mat, unsigned char *output, int size)
{
    if (mat->isContinuous())
    {
        memcpy(output, mat->data, size);
    }
    else
    {
        for (int i = 0; i < mat->rows; ++i)
        {
            int inserted = i * mat->cols * mat->channels();
            memcpy(output + inserted, mat->data + inserted, mat->cols * mat->channels());
        }
    }
}

void releaseMat(cv::Mat *mat)
{
    mat->release();
    free(mat);

}
