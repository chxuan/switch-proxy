#pragma once
/*
功能:udp流量统计
日期:2020-04-02
作者:chxuan <787280310@qq.com>
*/
#include <atomic>
#include <map>
#include <mutex>
/* #include "atimer_ex.h" */
/* #include "public/utility/log_file.h" */

const static int STATISTICS_SECONDS = 30 * 60;

class udp_flow_statistics
{
public:
    udp_flow_statistics() 
    {
        /* timer_.Bind([this] { print_network_flow(); }); */
        /* timer_.Start(STATISTICS_SECONDS); */
    }

public:
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

        /* LOG(LOGI_INFO, "统计间隔[%d]s%s", STATISTICS_SECONDS, str.c_str()); */
        packet_count_.clear();
    }

private:
    /* ATimerEx<boost::posix_time::seconds> timer_; */
    std::mutex mutex_;
    std::map<std::string, unsigned long long> packet_count_;
};

