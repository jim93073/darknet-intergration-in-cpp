#ifndef _YOLOTALK_UTILS_H_
#define _YOLOTALK_UTILS_H_
#define YOLOTALK_PREFIX "[yolotalk]"
#define YOLOTALK_CMD_PREFIX "[yolotalk_cmd]"

#define YOLOTALK_CMD_PREFIX_MODEL_FPS "[yolotalk_model_fps]"
#define YOLOTALK_CMD_PREFIX_VIDEO_FPS "[yolotalk_video_fps]"
#define YOLOTALK_CMD_PREFIX_QUEUE_SIZE "[yolotalk_queue_size]"

#define YOLOTALK_CMD_LOADING_MODEL "loading_model"
#define YOLOTALK_CMD_LOADING_MODEL_FINISH "loading_model_finish"
#define YOLOTALK_CMD_WARMING_UP "warming_up"
#define YOLOTALK_CMD_WARMING_UP_FINISH "warming_up_finish"
#define YOLOTALK_CMD_LOADING_VIDEO "loading_video"
#define YOLOTALK_CMD_LOADING_VIDEO_FINISH "loading_video_finish"
#define YOLOTALK_CMD_START_PREDICT "start_predict"
#define YOLOTALK_CMD_PREDICT_NO_OBJECT "predict_no_object"
#define YOLOTALK_CMD_VIDEO_CLOSED "video_closed"
#define YOLOTALK_CMD_PROGRAM_EXITED "program_exited"
#define YOLOTALK_CMD_QUEUE_OVERFLOW "queue_overflow"

#define YOLOTALK_CMD_ERROR_OPEN_VIDEO "error_open_video"
#define YOLOTALK_CMD_ERROR_READ_VIDEO_FRAME "error_read_video_frame"
#define YOLOTALK_CMD_ERROR_INVALID_VERTEX_PARAMETER "error_invalid_vertex_parameter"
#define YOLOTALK_CMD_ERROR_INVALID_OUTPUT_DIR "error_invalid_output_dir"

#include <vector>

namespace yolotalk
{
    typedef struct Point
    {
        float x, y;
    } Point;
} // namespace yolotalk

void print_msg(const char *__restrict __fmt, ...);
void print_msg_stderr(const char *__restrict __fmt, ...);
void print_cmd_stderr(const char *__restrict __fmt, ...);
void print_stderr(const char *__restrict __fmt, ...);
void print_stdout(const char *__restrict __fmt, ...);
void print_cmd(char *cmd);
void parse_vertex(std::vector<yolotalk::Point> *result, char *vertex_char);
int ensure_dir(char* path);

const std::string DateTime();
const std::string TIME();
const std::string Date();

template<typename T, typename P>
T remove_if(T beg, T end, P pred)
{
    T dest = beg;
    for (T itr = beg;itr != end; ++itr)
        if (!pred(*itr))
            *(dest++) = *itr;
    return dest;
}
#endif