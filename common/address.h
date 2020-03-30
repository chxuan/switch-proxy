#pragma once
/*
功能:地址
日期:2019.11.29
作者:chengxuan
*/
#include <string>

struct address 
{
    address(const std::string& ip, unsigned short port) 
        : ip(ip), port(port)
    {
        addr = ip + ":" + std::to_string(port);
    }

    address(const std::string& addr)
    {
        std::string::size_type pos = addr.find(":");
        if (pos != std::string::npos)
        {
            ip = std::string(addr, 0, pos);
            port = std::stoi(std::string(addr, pos + 1));
            this->addr = addr;
        }
    }

    std::string ip;
    unsigned short port = 0;
    std::string addr;
};
