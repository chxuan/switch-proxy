#pragma once
/*
功能:tcp转换代理会话管理
日期:2019.11.29
作者:chengxuan
*/
#include "../common/address.h"
#include "../common/io_service_pool.h"

class tcp_switch_session;

class tcp_switch_session_manager 
{
public:
    tcp_switch_session_manager(const address& listen_address, const std::vector<address>& target_address_list, int io_threads);
    ~tcp_switch_session_manager() {}

public:
    // 运行tcp代理会话管理器
    void run();

private:
    // 开始监听
    void start_accept();

private:
    io_service_pool io_service_pool_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::vector<address> target_address_list_;
};
