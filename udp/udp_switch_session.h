#pragma once

#include "udp_socket.h"

class udp_switch_session_manager;

class udp_switch_session
{
public:
    udp_switch_session(boost::asio::io_service& ios, const std::vector<address>& target_address_list, const address& client_address, udp_switch_session_manager* session_manager);

public:
    // 发送数据到target
    void send_to_target(const UdpBuffer& buffer, std::size_t len);
    // 异步接收target数据
    void async_receive_target();

private:
    // 重置保活定时器
    void reset_keepalive_timer();
    // 关闭udp代理会话
    void close();

private:
    boost::asio::io_service& ios_;
    boost::asio::deadline_timer keepalive_timer_;
    udp_socket target_socket_;
    address client_address_;
    udp_switch_session_manager* session_manager_;
    std::vector<address> target_address_list_;
    bool closed_ = false;
};

