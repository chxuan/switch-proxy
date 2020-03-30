#pragma once
/*
功能:网络转换代理服务
日期:2019.12.02
作者:chengxuan
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

