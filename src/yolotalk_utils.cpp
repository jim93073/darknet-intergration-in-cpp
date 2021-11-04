#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "yolotalk_utils.h"

int _mkdir(const char *dir);

void print_msg(const char *__restrict __fmt, ...)
{
    fprintf(stdout, "%s ", YOLOTALK_PREFIX);
    va_list args;
    va_start(args, __fmt);
    vfprintf(stdout, __fmt, args);
    va_end(args);
    fprintf(stdout, "%s", "\n");
}

void print_msg_stderr(const char *__restrict __fmt, ...)
{
    fprintf(stderr, "%s ", YOLOTALK_PREFIX);
    va_list args;
    va_start(args, __fmt);
    vfprintf(stderr, __fmt, args);
    va_end(args);
    fprintf(stderr, "%s", "\n");
}

void print_cmd_stderr(const char *__restrict __fmt, ...)
{
    fprintf(stderr, "%s ", YOLOTALK_CMD_PREFIX);
    va_list args;
    va_start(args, __fmt);
    vfprintf(stderr, __fmt, args);
    va_end(args);
    fprintf(stderr, "%s", "\n");
}

void print_stderr(const char *__restrict __fmt, ...)
{
    va_list args;
    va_start(args, __fmt);
    vfprintf(stderr, __fmt, args);
    va_end(args);
}

void print_stdout(const char *__restrict __fmt, ...)
{
    va_list args;
    va_start(args, __fmt);
    vfprintf(stdout, __fmt, args);
    va_end(args);
}

void print_cmd(char *cmd){
    fprintf(stderr, "%s %s\n", YOLOTALK_CMD_PREFIX, cmd);
}

void parse_vertex(std::vector<yolotalk::Point> *result, char *vertex_char)
{
    std::string vertex_str(vertex_char);

    // remove space
    remove_if(vertex_str.begin(), vertex_str.end(), isspace);

    std::string delimiter = ",";
    std::vector<float> coords;

    // split string
    size_t pos = 0;
    std::string token;
    while ((pos = vertex_str.find(delimiter)) != std::string::npos)
    {
        token = vertex_str.substr(0, pos);
        coords.push_back(std::stof(token));
        vertex_str.erase(0, pos + delimiter.length());
    }

    token = vertex_str.substr(0, pos);
    coords.push_back(std::stof(token));

    for (int i = 0; i < coords.size() - 1; i += 2)
    {
        if (i < coords.size() && i + 1 < coords.size())
        {
            yolotalk::Point p;
            p.x = coords[i];
            p.y = coords[i + 1];
            result->push_back(p);
        }
        else
        {
            throw "Error. Invalid vertex format.";
        }
    }

    if (result->size() < 3)
    {
        throw "Error. The number of vertices should large than 3.";
    }
}

int _mkdir(const char *dir)
{
    // Source: https://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
    char tmp[256];
    char *p = NULL;
    size_t len;

    int flag = 0;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/')
        {
            *p = 0;
            int f = mkdir(tmp, S_IRWXU);
            if ((flag == 0 || flag == -1) && f != 0)
            {
                flag = f;
            }
            *p = '/';
        }
    int f = mkdir(tmp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if ((flag == 0 || flag == -1) && f != 0)
    {
        flag = f;
    }
    return flag;
}

int ensure_dir(char *path)
{
    // return 1 if success
    struct stat info;

    if (stat(path, &info) != 0)
    {
        int flag = _mkdir(path);
        return (flag == 0 || flag == -1);
    }
    else if (info.st_mode & S_IFDIR)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

const std::string DateTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    tstruct.tm_sec -= 16;
    strftime(buf, sizeof(buf), "%Y-%m-%d-%H-%M-%S", &tstruct);
    return buf;
}

const std::string TIME()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    //tstruct.tm_sec -= 16;
    strftime(buf, sizeof(buf), "%H", &tstruct);
    return buf;
}

const std::string Date()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    //strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return buf;
}
