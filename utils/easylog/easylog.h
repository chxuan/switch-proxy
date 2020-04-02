#pragma once
/*
功能:日志库
日期:2018-06-22
作者:chxuan <787280310@qq.com>
*/
#include "log_file_proxy.h"
#include "format.h"

// 日志等级
enum class log_level { all = 0, trace, debug, info, warn, error, fatal };

class easylog
{
public:
    // 获取单例对象
    static easylog& get() { static easylog inst; return inst; }
    // 初始化easylog
    inline void init_easylog(const std::string& output_dir, log_level level, unsigned long long max_file_size);
    // 动态更改日志等级
    void modify_log_level(log_level level) { level_ = level; }
    // 输出日志到控制台和文件
    template<typename... Args>
    inline void log(log_level level, const char* file_name, unsigned long line, const char* fmt, Args&&... args);
    // 输出日志到文件
    template<typename... Args>
    inline void log_file(log_level level, const char* file_name, unsigned long line, const char* fmt, Args&&... args);
    // 输出日志到控制台
    template<typename... Args>
    inline void log_console(log_level level, const char* file_name, unsigned long line, const char* fmt, Args&&... args);

private:
    easylog() {}
    ~easylog() {}

    // 判断是否输出日志
    bool is_logged(log_level level) { return level >= level_; }
    // 创建日志
    inline std::string create_log(log_level level, const char* file_name, unsigned long line, const std::string& content);
    // 将日志等级转换成字符串
    inline const char* to_string(log_level level);
    // 写到日志文件
    inline void write_console(log_level level, const std::string& log);
    // 写到日志文件
    inline void write_file(log_level level, const std::string& log);

private:
    log_level level_;
    log_file_proxy all_proxy_;
    log_file_proxy warn_proxy_;
    log_file_proxy error_proxy_;
    log_file_proxy fatal_proxy_;
};

void easylog::init_easylog(const std::string& output_dir, log_level level, unsigned long long max_file_size)
{
    mkdir(output_dir);
    level_ = level;

    all_proxy_.init_log_file(output_dir, "ALL", max_file_size);
    warn_proxy_.init_log_file(output_dir, "WARN", max_file_size);
    error_proxy_.init_log_file(output_dir, "ERROR", max_file_size);
    fatal_proxy_.init_log_file(output_dir, "FATAL", max_file_size);
}

template<typename... Args>
void easylog::log(log_level level, const char* file_name, unsigned long line, const char* fmt, Args&&... args)
{
    if (is_logged(level))
    {
        std::string log = create_log(level, file_name, line, format(fmt, std::forward<Args>(args)...));
        write_console(level, log);
        write_file(level, log);
    }
}

template<typename... Args>
void easylog::log_file(log_level level, const char* file_name, unsigned long line, const char* fmt, Args&&... args)
{
    if (is_logged(level))
    {
        std::string log = create_log(level, file_name, line, format(fmt, std::forward<Args>(args)...));
        write_file(level, log);
    }
}

template<typename... Args>
void easylog::log_console(log_level level, const char* file_name, unsigned long line, const char* fmt, Args&&... args)
{
    if (is_logged(level))
    {
        std::string log = create_log(level, file_name, line, format(fmt, std::forward<Args>(args)...));
        write_console(level, log);
    }
}

std::string easylog::create_log(log_level level, const char* file_name, unsigned long line, const std::string& content)
{
    std::string log;
    log += to_string(level);
    log += get_current_time_ms();
    log += " ";
    log += file_name;
    log += ":";
    log += std::to_string(line);
    log += "] ";
    log += content;
    log += "\n";

    return log;
}

const char* easylog::to_string(log_level level)
{
    switch (level)
    {
    case log_level::all: return "A";
    case log_level::trace: return "T";
    case log_level::debug: return "D";
    case log_level::info: return "I";
    case log_level::warn: return "W";
    case log_level::error: return "E";
    case log_level::fatal: return "F";
    default: return "U";
    }
}

void easylog::write_console(log_level level, const std::string& log)
{
    switch (level)
    {
    case log_level::warn: printf("\033[1;33m%s\033[0m", log.c_str()); break;
    case log_level::error: printf("\033[1;31m%s\033[0m", log.c_str()); break;
    case log_level::fatal: printf("\033[1;35m%s\033[0m", log.c_str()); break;
    default: printf("%s", log.c_str()); break;
    }
}

void easylog::write_file(log_level level, const std::string& log)
{
    if (level == log_level::warn)
    {
        warn_proxy_.write_log(log);
    }
    else if (level == log_level::error)
    {
        error_proxy_.write_log(log);
    }
    else if (level == log_level::fatal)
    {
        fatal_proxy_.write_log(log);
    }

    all_proxy_.write_log(log);
}

#define FILE_LINE basename(const_cast<char*>(__FILE__)), __LINE__

// 初始化easylog
#define EASYLOG_INIT(output_dir, level, max_file_size) easylog::get().init_easylog(output_dir, level, max_file_size)
// 动态更改日志等级
#define EASYLOG_MODIFY_LOG_LEVEL(level) easylog::get().modify_log_level(level)

// 输出日志到控制台和文件
#define LOG_ALL(...) easylog::get().log(log_level::all, FILE_LINE, __VA_ARGS__)
#define LOG_TRACE(...) easylog::get().log(log_level::trace, FILE_LINE, __VA_ARGS__)
#define LOG_DEBUG(...) easylog::get().log(log_level::debug, FILE_LINE, __VA_ARGS__)
#define LOG_INFO(...) easylog::get().log(log_level::info, FILE_LINE, __VA_ARGS__)
#define LOG_WARN(...) easylog::get().log(log_level::warn, FILE_LINE, __VA_ARGS__)
#define LOG_ERROR(...) easylog::get().log(log_level::error, FILE_LINE, __VA_ARGS__)
#define LOG_FATAL(...) easylog::get().log(log_level::fatal, FILE_LINE, __VA_ARGS__)

// 输出日志到控制台
#define LOG_CONSOLE_ALl(...) easylog::get().log_console(log_level::all, FILE_LINE, __VA_ARGS__)
#define LOG_CONSOLE_TRACE(...) easylog::get().log_console(log_level::trace, FILE_LINE, __VA_ARGS__)
#define LOG_CONSOLE_DEBUG(...) easylog::get().log_console(log_level::debug, FILE_LINE, __VA_ARGS__)
#define LOG_CONSOLE_INFO(...) easylog::get().log_console(log_level::info, FILE_LINE, __VA_ARGS__)
#define LOG_CONSOLE_WARN(...) easylog::get().log_console(log_level::warn, FILE_LINE, __VA_ARGS__)
#define LOG_CONSOLE_ERROR(...) easylog::get().log_console(log_level::error, FILE_LINE, __VA_ARGS__)
#define LOG_CONSOLE_FATAL(...) easylog::get().log_console(log_level::fatal, FILE_LINE, __VA_ARGS__)

// 输出日志到文件
#define LOG_FILE_ALL(...) easylog::get().log_file(log_level::all, FILE_LINE, __VA_ARGS__)
#define LOG_FILE_TRACE(...) easylog::get().log_file(log_level::trace, FILE_LINE, __VA_ARGS__)
#define LOG_FILE_DEBUG(...) easylog::get().log_file(log_level::debug, FILE_LINE, __VA_ARGS__)
#define LOG_FILE_INFO(...) easylog::get().log_file(log_level::info, FILE_LINE, __VA_ARGS__)
#define LOG_FILE_WARN(...) easylog::get().log_file(log_level::warn, FILE_LINE, __VA_ARGS__)
#define LOG_FILE_ERROR(...) easylog::get().log_file(log_level::error, FILE_LINE, __VA_ARGS__)
#define LOG_FILE_FATAL(...) easylog::get().log_file(log_level::fatal, FILE_LINE, __VA_ARGS__)

