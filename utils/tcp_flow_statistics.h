#pragma once
/*
功能:tcp流量统计
日期:2020-04-02
作者:chxuan <787280310@qq.com>
*/
#include <atomic>
#include <map>
#include <mutex>
#include "task_timer.h"
#include "./easylog/easylog.h"

const static int STATISTICS_SECONDS = 30 * 60;

class tcp_flow_statistics
{
public:
    tcp_flow_statistics() 
    {
        timer_.bind([this] { print_network_flow(); });
        timer_.start(STATISTICS_SECONDS);
    }

public:
    // 增加一个连接
    void increment_connection()
    {
        ++count_;
        print_connection_count();
    }

    // 减少一个连接
    void subtract_connection()
    {
        --count_;
        print_connection_count();
    }

    // 增加一个数据包
    void increment_packet(const std::string& description)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        ++packet_count_[description];
    }

private:
    // 打印网络流量
    void print_network_flow()
    {
        print_connection_count();
        print_packet_count();
    }

    // 打印连接数量
    void print_connection_count()
    {
        LOG_INFO("当前连接数:count[{}]", count_.load());
    }

    // 打印网络收发包
    void print_packet_count()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        std::string str;
        for (auto& iter : packet_count_)
        {
            str += "--" + iter.first + "[" + std::to_string(iter.second) + "]";
        }

        if (str.empty())
        {
            str = "没有收到任何网络包";
        }

        LOG_INFO("统计间隔[{}]s{}", STATISTICS_SECONDS, str);
        packet_count_.clear();
    }

private:
    task_timer<boost::posix_time::seconds> timer_;
    std::atomic<int> count_{0};
    std::mutex mutex_;
    std::map<std::string, unsigned long long> packet_count_;
};

