#pragma once
/*
功能:工具
日期:2018-06-23
作者:chxuan <787280310@qq.com>
*/
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <string>

namespace el
{
namespace util
{

// 获得当前可执行文件名
inline std::string get_current_exe_name()
{
    char buf[1024] = {'\0'};

    if (readlink("/proc/self/exe", buf, sizeof(buf)) != -1)
    {
        return basename(buf);
    }

    return "";
}

// 获取当前时间,精确到毫秒 2018-06-23 16:39:50.131
inline std::string get_current_time_ms()
{
    struct timeval now_tv;
    gettimeofday(&now_tv, nullptr);

    struct tm t;
    localtime_r(&now_tv.tv_sec, &t);

    char str[24] = {"\0"};
    snprintf(str, sizeof(str), "%04d-%02d-%02d %02d:%02d:%02d.%03d", 
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, 
             t.tm_sec, static_cast<int>(now_tv.tv_usec / 1000));

    return str;
}

// 判断文件是否存在
inline bool is_exists(const std::string& path)
{
    // F_OK 用于判断文件是否存在
    return access(path.c_str(), F_OK) != -1;
}

// 创建多级目录
inline bool mkdir(const std::string& path)
{
    if (is_exists(path))
    {
        return true;
    }

    std::string dir = path;
    if (dir[dir.size() - 1] != '/')
    {
        dir += '/';
    }

    for (std::size_t i = 0; i < dir.size(); ++i)
    {
        if (dir[i] == '/' && i != 0) 
        {
            std::string sub_dir = dir.substr(0, i);
            if (!is_exists(sub_dir))
            {
                if (::mkdir(sub_dir.c_str(), 0775) == -1)
                {
                    return false;
                }
            }
        }
    }

    return true;
}


}
}

