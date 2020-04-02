#pragma once
/*
功能:网络转换代理
日期:2020-04-02
作者:chxuan <787280310@qq.com>
*/
#include <vector>
#include "../common/address.h"

class switch_proxy
{
public:
    // 启动转换代理服务
    void run();

private:
    // 读取目标服务地址
    std::vector<address> read_target_address();
};

