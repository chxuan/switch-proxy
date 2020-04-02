#pragma once
/*
功能:tcp转换代理会话
日期:2020-04-02
作者:chxuan <787280310@qq.com>
*/
#include "tcp_socket.h"

class tcp_switch_session : public std::enable_shared_from_this<tcp_switch_session>
{
public:
    tcp_switch_session(boost::asio::io_service& ios);

public:
    // 获得client的socket
    tcp_socket& get_client_socket();
    // 启动tcp代理会话
    void start(const std::vector<address>& target_address_list);

private:
    // 初始化目标服务socket
    void init_target_socket(const std::vector<address>& target_address_list);
    // 连接目标服务
    bool connect_target_server();

    // 异步读取client的数据
    void async_read_client();
    // 异步读取target的数据
    void async_read_target();
    // 异步读取target的数据
    void async_read_target(tcp_socket& target);

    // 发送数据到client
    void send_to_client(tcp_socket& target, std::size_t len);
    // 发送数据到target服务
    void send_to_target(std::size_t len);

    // 重置保活定时器
    void reset_keepalive_timer();
    // 关闭tcp代理会话
    void close();
    // 关闭target socket
    void close_target_socket();
    // 关闭client socket
    void close_client_socket();

private:
    boost::asio::io_service& ios_;
    boost::asio::deadline_timer keepalive_timer_;
    tcp_socket client_socket_;
    std::vector<tcp_socket> target_socket_list_;
    bool closed_ = false;
};
