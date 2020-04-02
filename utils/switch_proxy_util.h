#pragma once
/*
功能:工具
日期:2020-04-02
作者:chxuan <787280310@qq.com>
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>

namespace sp
{
namespace util
{

// 读取文件
inline std::vector<std::string> read_file(const std::string& file_path)
{
    std::vector<std::string> results;

    std::ifstream file;
    file.open(file_path);

    std::string line;
    while (getline(file, line))
    {
        results.emplace_back(line);
    }
    file.close();

    return results;
}

// 字符串分割
inline std::vector<std::string> split(const std::string& str, const std::string& delimiter)
{
    std::string save_str = str;
    char* save = nullptr;
    char* token = strtok_r(const_cast<char*>(save_str.c_str()), delimiter.c_str(), &save);

    std::vector<std::string> results;
    while (token != nullptr)
    {
        results.emplace_back(token);
        token = strtok_r(nullptr, delimiter.c_str(), &save);
    }
    return results;
}

// 查找字符串
inline bool contains(const std::string& str, const std::string& token)
{
	return str.find(token) == std::string::npos ? false : true;
}

// 二进制转换成十六进制字符串
inline std::string bcd_to_hex(const unsigned char* bcd, int len, const std::string& separator = "-")
{
    auto get_char = [](int value) 
    {
        if (value < 10)
        {
            return value + 48;
        }
        else
        {
            return value + 65 - 10;
        }
    };

    std::string result;
    for (int i = 0; i < len; i++)
    {
        if (result.length())
        {
            result.append(separator);
        }
        result.append(1, get_char(bcd[i] >> 4));
        result.append(1, get_char(bcd[i] % 16));
    }

    return result;
}

// 二进制转换成十六进制字符串
inline std::string bcd_to_hex(const std::string& bcd, const std::string& separator = "-")
{
    return bcd_to_hex(reinterpret_cast<const unsigned char*>(bcd.c_str()), bcd.length(), separator);
}

}
}

