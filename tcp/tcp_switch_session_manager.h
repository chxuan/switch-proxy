#pragma once
/*
功能:tcp转发代理会话管理
日期:2020-04-02
作者:chxuan <787280310@qq.com>
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
