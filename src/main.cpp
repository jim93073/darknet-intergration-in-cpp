#include <stdlib.h>
#include <stdio.h>
#include <chrono>


#include <utils.h>
#include <data.h>
#include <box.h>
#include <image_utils.h>

#include <libyolotalk.h>
#include <unistd.h>
#include <darknet.h>
#define MAX_QUEUE_BUFFER 180

enum CoordinateStyle
{
    CENTER_POINT, // only display the center coordinate of the bounding box
    MIN_MAX_X_Y   // display the bounding box with min X, min Y, max X and max Y
};

void print_bboxes(std::vector<BoundingBox> boxes, int frame_id = -1, char *out_path=NULL, bool enablePrefix = true, bool displayClassName = true, bool displayConfidence = true, CoordinateStyle style = MIN_MAX_X_Y, int classId = -1);
void draw_bboxes(Mat *frame, std::vector<BoundingBox> boxes, char *filename);
void predictionListener(int frame_id, Mat *mat, BoundingBox **bboxes, int bbox_len, char* file_path);

static YoloDevice *device = NULL;

int main(int argc, char **argv)
{
    // Parse args
    char *cfg = find_char_arg(argc, argv, "-cfg", "../weights/yolov4.cfg");
    char *weights = find_char_arg(argc, argv, "-weights", "../weights/yolov4.weights");
    char *name_list = find_char_arg(argc, argv, "-names", "../weights/coco.names");
    char *url = find_char_arg(argc, argv, "-url", "rtsp://iottalk:iottalk2019@140.113.237.220:554/live2.sdp");
    float thresh = find_float_arg(argc, argv, "-thresh", 0.25);
    int enable_polygon = find_arg(argc, argv, "-use_polygon");
    char *output_folder = find_char_arg(argc, argv, "-output_dir", NULL);

    // check output directory
    if (output_folder != NULL)
    {
        if(!ensure_dir(output_folder))
        {
            print_msg_stderr("Error. Invalid output dir path: \"%s\"", output_folder);
            print_cmd(YOLOTALK_CMD_ERROR_INVALID_OUTPUT_DIR);
            exit(1);
        }
        std::string output_folder_str = std::string(output_folder);
        if (output_folder_str[output_folder_str.length()-1] != '/' )
        {
            output_folder_str = output_folder_str + "/";
            output_folder = (char*) calloc(output_folder_str.size()+1, sizeof(char));
            strcpy(output_folder, output_folder_str.c_str());
        }
    }

    // parse vertice of the polygen
    std::vector<yolotalk::Point> vertex_points;
    if (enable_polygon)
    {
        char *vertex = find_char_arg(argc, argv, "-vertex", NULL);

        try
        {
            parse_vertex(&vertex_points, vertex);
        }
        catch (char const *error)
        {
            print_msg_stderr("Error. Invalid vertex format.", "");
            print_cmd(YOLOTALK_CMD_ERROR_INVALID_VERTEX_PARAMETER);
            printf("%s\n", error);
            exit(1);
        }
    }

    device = new YoloDevice(cfg, weights, name_list, url, thresh, NULL, 0, output_folder, MAX_QUEUE_BUFFER, true);
    device->setPolygon(&vertex_points);

    device->setPredictionListener(predictionListener);

    device->start();

    // while (true)
    // {
    //     std::cout << device.getFps() << std::endl;
    //     usleep(500000);
    // }

    device->join();

    print_msg_stderr("%s", "Program closed.");
    print_cmd(YOLOTALK_CMD_PROGRAM_EXITED);

    return 0;
}

void predictionListener(int frame_id, Mat *mat, BoundingBox **bboxes, int bbox_len, char* file_path)
{
    std::vector<BoundingBox> v_bboxes;
    for(int i = 0; i < bbox_len; i++)
    {
        v_bboxes.push_back(*(bboxes[i]));
    }
    print_bboxes(v_bboxes, frame_id, file_path);
    if(file_path != NULL)
    {
        // draw_bboxes(mat, v_bboxes, file_path);
    }
}

void print_bboxes(std::vector<BoundingBox> boxes, int frame_id, char *out_path, bool enablePrefix, bool displayClassName, bool displayConfidence, CoordinateStyle style, int classId)
{
    if (boxes.size() == 0)
    {
        // No object was detected
        print_cmd(YOLOTALK_CMD_PREDICT_NO_OBJECT);
        print_stdout("%s %s\n", YOLOTALK_CMD_PREFIX, YOLOTALK_CMD_PREDICT_NO_OBJECT);
        fflush(stdout);
        return;
    }

    for (int i = 0; i < boxes.size(); i++)
    {
        BoundingBox bbox = boxes[i];

        if (classId != -1 && classId != bbox.getClassId())
        {
            continue;
        }

        if (enablePrefix)
        {
            print_stdout("%s ", YOLOTALK_CMD_PREFIX);
        }

        if (frame_id > -1)
        {
            print_stdout("%d ", frame_id);
        }

        if (displayClassName)
        {
            print_stdout("%s ", bbox.getName());
        }

        if (displayConfidence)
        {
            print_stdout("%.03f ", bbox.getConfidence());
        }

        if (style == CoordinateStyle::MIN_MAX_X_Y)
        {
            int minX = bbox.getXmin();
            int minY = bbox.getYmin();
            int maxX = bbox.getXmax();
            int maxY = bbox.getYmax();
            print_stdout("%d %d %d %d", minX, minY, maxX, maxY);
        }
        else if (style == CoordinateStyle::CENTER_POINT)
        {
            print_stdout("%d %d", (int)round(bbox.getBox().x), (int)round(bbox.getBox().y));
        }

        if (out_path != NULL)
        {
            printf(" %s", out_path);
        }

        print_stdout("\n");
        fflush(stdout);
    }
}

void draw_bboxes(Mat *frame, std::vector<BoundingBox> boxes, char *filename)
{
    float red = 0;
    float green = 0;
    float blue = 0;

    for (int i = 0; i < boxes.size(); i++)
    {
        BoundingBox bbox = boxes[i];
        int left = bbox.getXmin();
        int top = bbox.getYmin();
        int right = bbox.getXmax();
        int bottom = bbox.getYmax();
        printf("%s: %.0f%%", bbox.getName(), bbox.getConfidence());
        printf("\t(left: %d   right: %d   top: %d   bot: %d)\n", left, right, top, bottom);
        
        device->getColors(bbox.getClassId(), &red, &green, &blue);

        printf("%f %f %f\n", red, green, blue);

        rectangle(*frame, Point(left, top), Point(right, bottom), Scalar(blue, green, red), LINE_4, LINE_4);
        putText(*frame, bbox.getName(), Point(left, top - 5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(blue, green, red), 2);
    }

    imwrite(filename, *frame);
}
