#pragma once
/*
功能:配置文件
日期:2020-04-02
作者:chxuan <787280310@qq.com>
*/
#include <unordered_map>
#include "switch_proxy_util.h"

using namespace sp::util;

class config_file
{
public:
    // 设置文件路径
    void set_file_path(const std::string& file_path)
    {
        std::vector<std::string> lines = read_file(file_path);
        for (auto& line : lines)
        {
            if (!contains(line, "#") && contains(line, "="))
            {
                std::vector<std::string> arr = split(line, "=");
                if (arr.size() == 2)
                {
                    configs_.emplace(arr[0], arr[1]);
                }
            }
        }
    }

    // 获得string类型value
    std::string get_string(const std::string& key)
    {
        auto iter = configs_.find(key);
        if (iter != configs_.end())
        {
            return iter->second;
        }

        return "";
    }

    // 获得int类型value
    int get_int(const std::string& key)
    {
        return atoi(get_string(key).c_str());
    }

    // 获得long long类型value
    long long get_long_long(const std::string& key)
    {
        return atoll(get_string(key).c_str());
    }

    // 判断key是否存在
    bool is_exists(const std::string& key)
    {
        return configs_.find(key) != configs_.end();
    }

private:
    std::unordered_map<std::string, std::string> configs_;
};

