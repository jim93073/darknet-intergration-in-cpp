#include "libyolotalk.h"
#include <string.h>

char *create_str(char *str_in)
{
    if (str_in == NULL)
    {
        return NULL;
    }
    int size = strlen(str_in) + 1;
    char *str_out = (char *) calloc(size, sizeof(char));
    memset(str_out, '\0', size);
    strcpy(str_out, str_in);
    return str_out;
}

extern "C"
{
    BoundingBox *BoundingBox_new(char *name, int classId, float confidence, box boxIn) { return new BoundingBox(name, classId, confidence, boxIn); }
    int BoundingBox_getXmin(BoundingBox *bbox) { return bbox->getXmin(); }
    int BoundingBox_getXmax(BoundingBox *bbox) { return bbox->getXmax(); }
    int BoundingBox_getYmin(BoundingBox *bbox) { return bbox->getYmin(); }
    int BoundingBox_getYmax(BoundingBox *bbox) { return bbox->getYmax(); }
    int BoundingBox_getClassId(BoundingBox *bbox) { return bbox->getClassId(); }
    char *BoundingBox_getName(BoundingBox *bbox) { return bbox->getName(); }
    float BoundingBox_getConfidence(BoundingBox *bbox) { return bbox->getConfidence(); }
    box BoundingBox_getBox(BoundingBox *bbox) { return bbox->getBox(); }

    YoloDevice *YoloDevice_new(char *cfg, char *weights, char *name_list, char *url, float thresh, float *vertices, int vertices_size, char *output_folder, int max_video_queue_size, bool show_msg)
    {
        char *_cfg = create_str(cfg);
        char *_weights = create_str(weights);
        char *_name_list = create_str(name_list);
        char *_url = create_str(url);
        char *_output_folder = create_str(output_folder);

        return new YoloDevice(_cfg, _weights, _name_list, _url, thresh, vertices, vertices_size, _output_folder, max_video_queue_size, show_msg);
    };
    void YoloDevice_start(YoloDevice *device) { device->start(); }
    void YoloDevice_setPolygon(YoloDevice *device, float *vertices, int vertices_size) { device->setPolygon(vertices, vertices_size); }
    void YoloDevice_setPredictionListener(YoloDevice *device, void (*func)(int frame_id, Mat *mat, BoundingBox **bboxes, int bbox_len, char *file_path)) { device->setPredictionListener(func); }
    void YoloDevice_join(YoloDevice *device) { device->join(); }
    void YoloDevice_stop(YoloDevice *device, bool force) { device->stop(force); }
    float YoloDevice_getVideoFps(YoloDevice *device) { return device->getVideoFps(); }
    float YoloDevice_getModelFps(YoloDevice *device) { return device->getModelFps(); }
    float YoloDevice_getFps(YoloDevice *device) { return device->getFps(); }
    void YoloDevice_getColors(YoloDevice *device, int class_id, float *return_r, float *return_g, float *return_b) { device->getColors(class_id, return_r, return_g, return_b); }
    yolotalk::Point* YoloDevice_getPolygon(YoloDevice *device, int *vertices_num)
    {
        if (device->getPolygon() == NULL)
        {
            return NULL;
        }
        std::vector<yolotalk::Point> vertices = *device->getPolygon();
        *vertices_num = vertices.size();
        return device->getPolygonData();
    };

    void releaseMat(cv::Mat *mat);
    void matToArray(cv::Mat *mat, unsigned char *output, int size);
    int getMatLength(cv::Mat *mat);
    void getMatInfo(cv::Mat *mat, int *rows, int *cols, int *channels);
}