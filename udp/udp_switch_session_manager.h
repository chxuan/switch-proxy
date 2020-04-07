#pragma once
/*
功能:udp转发代理会话管理
日期:2020-04-02
作者:chxuan <787280310@qq.com>
*/
#include <unordered_map>
#include <mutex>
#include "../common/io_service_pool.h"
#include "udp_socket.h"

class udp_switch_session;

class udp_switch_session_manager 
{
public:
    udp_switch_session_manager(const address& listen_address, const std::vector<address>& target_address_list, int io_threads);

public:
    // 运行udp代理会话管理器
    void run();
    // 发送数据到client
    void send_to_client(udp_socket& target_socket, std::size_t len, const address& client_address);
    // udp会话代理已经关闭通知
    void udp_switch_session_closed(const std::string& client_address);

private:
    // 异步接收client的数据
    void async_receive_client();
    // 获取或创建udp代理会话
    std::shared_ptr<udp_switch_session> get_or_create_udp_switch_session(const std::string& client_address);

private:
    io_service_pool io_service_pool_;
    udp_socket client_socket_;
    std::vector<address> target_address_list_;

    std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<udp_switch_session>> udp_switch_session_cache_;
};
