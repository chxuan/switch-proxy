#pragma once
/*
功能:io service池
日期:2020-04-02
作者:chxuan <787280310@qq.com>
*/
#include <boost/asio.hpp>
#include <vector>

class io_service_pool
{
public:
    io_service_pool(int pool_size);

public:
    // 运行io service池
    void run();
    // 运行io service池
    void stop();
    // 获得io service
    boost::asio::io_service& get_io_service();

private:
    std::vector<std::shared_ptr<boost::asio::io_service>> ios_pool_;
    std::vector<std::shared_ptr<boost::asio::io_service::work>> work_pool_;
    std::size_t next_io_service_ = 0;
};
