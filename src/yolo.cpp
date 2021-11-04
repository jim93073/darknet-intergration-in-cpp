#include "yolo.h"

#include <darknet.h>
#include <data.h>
#include <box.h>
#include <parser.h>
#include <image_utils.h>
#include <polygen_utils.h>

int compare_by_lefts(const void *a_ptr, const void *b_ptr);

BoundingBox::BoundingBox(char* name, int classId, float confidence, box boxIn)
{
    this->name = name;
    this->classId = classId;
    this->confidence = confidence;
    this->boxIn = boxIn;
}

int BoundingBox::getXmin()
{
    return round((this->boxIn.x - (this->boxIn.w / 2)));
}

int BoundingBox::getXmax()
{
    return round((this->boxIn.x + (this->boxIn.w / 2)));;
}

int BoundingBox::getYmin()
{
    return round((this->boxIn.y - (this->boxIn.h / 2)));
}

int BoundingBox::getYmax()
{
    return round((this->boxIn.y + (this->boxIn.h / 2)));
}

char* BoundingBox::getName()
{
    return this->name;
}

float BoundingBox::getConfidence()
{
    return this->confidence;
}

int BoundingBox::getClassId()
{
    return this->classId;
}

box BoundingBox::getBox()
{
    return this->boxIn;
}

Yolo::Yolo(char *cfg, char *weights, char *name_list)
{
    std::vector<yolotalk::Point> *vertex_points = new std::vector<yolotalk::Point>();
    this->init(cfg, weights, name_list, vertex_points);
}

Yolo::Yolo(char *cfg, char *weights, char *name_list, std::vector<yolotalk::Point> *vertex_points)
{
    this->init(cfg, weights, name_list, vertex_points);
}

Yolo::~Yolo()
{
    free_network(this->net);
}

void Yolo::init(char *cfg, char *weights, char *name_list, std::vector<yolotalk::Point> *vertex_points)
{
    this->names = get_labels_custom(name_list, &this->names_size);
    this->net = parse_network_cfg_custom(cfg, 1, 1);
    if (weights) {
        load_weights(&net, weights);
    }
    fuse_conv_batchnorm(net);
    calculate_binary_weights(net);
    srand(2222222);
    this->l = net.layers[net.n - 1];
    this->width = network_width(&net);
    this->height = network_height(&net);
    this->classes = this->l.classes;
    this->vertex_points = vertex_points;
}

void Yolo::setPolygon(std::vector<yolotalk::Point> *vertex_points)
{
    this->vertex_points = vertex_points;
}

std::vector<BoundingBox*> *Yolo::detect_image(Mat *frame, float thresh, float nms)
{
    // Mat frame_to_predict, frame_resized;
    // cvtColor(frame, frame_to_predict, COLOR_BGR2RGB);
    // resize(frame_to_predict, frame_resized, cv::Size(this->width, this->height));

    // image img = mat_to_image(frame_resized);
    image img = mat_to_image(*frame);
    image img_sized = resize_image(img, net.w, net.h);
    float *X = img_sized.data;

    network_predict(net, X);

    int nboxes = 0;
    detection *detections = get_network_boxes(&net, width, height, thresh, 0.5, NULL, 0, &nboxes, 0);

    if (nms)
    {
        if (l.nms_kind == DEFAULT_NMS)
            do_nms_sort(detections, nboxes, l.classes, nms);
        else
            diounms_sort(detections, nboxes, l.classes, nms, l.nms_kind, l.beta_nms);
    }

    int selected_detections_num;
    detection_with_class* selected_detections = get_actual_detections(detections, nboxes, thresh, &selected_detections_num, names);
    qsort(selected_detections, selected_detections_num, sizeof(*selected_detections), compare_by_lefts);

    std::vector<BoundingBox*> *results = new std::vector<BoundingBox*>();

    cv::Size frameSize = (*frame).size();
    int ori_w = frameSize.width;
    int ori_h = frameSize.height;

    float w_ratio = (float) ori_w / (float) width;
    float h_ratio = (float) ori_h / (float) height;

    for (int i = 0; i < selected_detections_num; ++i) {
        const int best_class = selected_detections[i].best_class;
        
        box b = selected_detections[i].det.bbox;
        b.h *= h_ratio;
        b.w *= w_ratio;
        b.x *= w_ratio;
        b.y *= h_ratio;

        bool can_output = true;
        // checking if point inside polygen
        if (this->vertex_points->size() > 0)
        {
            yolotalk::Point *polygen = &(*this->vertex_points)[0];
            yolotalk::Point p;
            p.x = b.x;
            p.y = b.y;
            if (!isInside(polygen, this->vertex_points->size(), p))
            {
                can_output = false;
            }
        }
        
        if (can_output)
        {
            BoundingBox *bbox = new BoundingBox(this->names[best_class], best_class, selected_detections[i].det.prob[best_class] * 100, b);
            results->push_back(bbox);
        }
    }

    // frame_to_predict.release();
    // frame_resized.release();

    free_image(img);
    free_image(img_sized);
    free(selected_detections);
    free_detections(detections, nboxes);

    return results;
}

int Yolo::getWidth()
{
    return this->width;
}

int Yolo::getHeight()
{
    return this->height;
}

int Yolo::getColorRed(int classId)
{
    int offset = classId * 123457 % l.classes;
    return get_color(2, offset, l.classes) * (float) 255;
}

int Yolo::getColorGreen(int classId)
{
    int offset = classId * 123457 % l.classes;
    return get_color(1, offset, l.classes) * (float) 255;
}

int Yolo::getColorBlue(int classId)
{
    int offset = classId * 123457 % l.classes;
    return get_color(0, offset, l.classes) * (float) 255;
}

int compare_by_lefts(const void *a_ptr, const void *b_ptr) {
    const detection_with_class* a = (detection_with_class*)a_ptr;
    const detection_with_class* b = (detection_with_class*)b_ptr;
    const float delta = (a->det.bbox.x - a->det.bbox.w/2) - (b->det.bbox.x - b->det.bbox.w/2);
    return delta < 0 ? -1 : delta > 0 ? 1 : 0;
}
