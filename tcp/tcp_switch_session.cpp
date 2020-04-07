#include "tcp_switch_session.h"
#include "../utils/tcp_flow_statistics.h"
#include "../utils/singletion.h"
#include "../utils/switch_proxy_util.h"

using namespace sp::util;

const static std::string RECEIVE_FROM_CLIENT = "receive_from_client";
const static std::string SEND_TO_CLIENT = "send_to_client";
const static std::string RECEIVE_FROM_TARGET = "receive_from_target";
const static std::string SEND_TO_TARGET = "send_to_target";

const static int KEEPALIVE_TIMEOUT_SECONDS = 8 * 60;

tcp_switch_session::tcp_switch_session(boost::asio::io_service& ios)
    : ios_(ios),
    keepalive_timer_(ios),
    client_socket_(ios)
{
}

tcp_socket& tcp_switch_session::get_client_socket()
{
    return client_socket_;
}

void tcp_switch_session::start(const std::vector<address>& target_address_list)
{
    LOG_ALL("接收到客户端连接:client[{}]", client_socket_.get_session_id());
    singletion<tcp_flow_statistics>::instance().increment_connection();

    reset_keepalive_timer();
    init_target_socket(target_address_list);
    if (connect_target_server())
    {
        async_read_client();
        async_read_target();
    }
}

void tcp_switch_session::init_target_socket(const std::vector<address>& target_address_list)
{
    for (std::size_t i = 0; i < target_address_list.size(); ++i)
    {
        target_socket_list_.emplace_back(tcp_socket(ios_, i, target_address_list[i]));
    }
}

bool tcp_switch_session::connect_target_server()
{
    for (auto& target : target_socket_list_)
    {
        try
        {
            target.socket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(target.remote_address.ip), target.remote_address.port));
            LOG_ALL("连接target{}成功:client[{}]--target{}[{}]", target.serial, client_socket_.get_session_id(), target.serial, target.get_session_id());
        }
        catch (std::exception& e)
        {
            LOG_WARN("连接target{}失败:client[{}]--target{}[{}]--error[{}]", target.serial, client_socket_.get_session_id(), target.serial, target.get_session_id(), e.what());
            close();
            return false;
        }
    }

    return true;
}

void tcp_switch_session::async_read_client()
{
    client_socket_.socket.async_read_some(boost::asio::buffer(client_socket_.buffer), 
                                          [this](const boost::system::error_code& ec, std::size_t len)
    {
        if (closed_)
        {
            return;
        }

        if (!ec)
        {
            reset_keepalive_timer();

            LOG_ALL("接收到client的数据:client[{}]--len[{}]--data[{}]", client_socket_.get_session_id(), len, bcd_to_hex(reinterpret_cast<const unsigned char*>(&client_socket_.buffer), len));
            singletion<tcp_flow_statistics>::instance().increment_packet(RECEIVE_FROM_CLIENT);

            send_to_target(len);
            async_read_client();
        }
        // boost::asio::error::operation_aborted
        // 正在async_read_some()异步任务等待时，本端关闭套接字
        else if (ec != boost::asio::error::operation_aborted)
        {
            LOG_WARN("接收client数据失败,关闭tcp代理:client[{}]--error[{}]", client_socket_.get_session_id(), ec.message());
            close();
        }
    });
}

void tcp_switch_session::async_read_target()
{
    for (auto& target : target_socket_list_)
    {
        async_read_target(target);
    }
}

void tcp_switch_session::async_read_target(tcp_socket& target)
{
    target.socket.async_read_some(boost::asio::buffer(target.buffer), 
                                  [this, &target](const boost::system::error_code& ec, std::size_t len)
    {
        if (closed_)
        {
            return;
        }

        if (!ec)
        {
            LOG_ALL("接收到target{}的数据:client[%s]--target{}[{}]--len[{}]--data[{}]", target.serial, client_socket_.get_session_id(), target.serial, target.get_session_id(), len, bcd_to_hex(reinterpret_cast<const unsigned char*>(&target.buffer), len));
            singletion<tcp_flow_statistics>::instance().increment_packet(RECEIVE_FROM_TARGET + std::to_string(target.serial));

            send_to_client(target, len);
            async_read_target(target);
        }
        // boost::asio::error::operation_aborted
        // 正在async_read_some()异步任务等待时，本端关闭套接字
        else if (ec != boost::asio::error::operation_aborted)
        {
            LOG_WARN("接收target{}数据失败:client[{}]--target{}[{}]--error[{}]", target.serial, client_socket_.get_session_id(), target.serial, target.get_session_id(), ec.message());
            close();
        }
    });
}

void tcp_switch_session::send_to_client(tcp_socket& target, std::size_t len)
{
    boost::system::error_code ec;
    std::size_t write_len = boost::asio::write(client_socket_.socket, boost::asio::buffer(target.buffer, len), ec);
    if (!ec)
    {
        LOG_ALL("向client写数据成功:client[{}]--target{}[{}]--len[{}]--data[{}]", client_socket_.get_session_id(), target.serial, target.get_session_id(), write_len, bcd_to_hex(reinterpret_cast<const unsigned char*>(&target.buffer), write_len));
        singletion<tcp_flow_statistics>::instance().increment_packet(SEND_TO_CLIENT);
    }
    else
    {
        LOG_ALL("向client写数据失败:client[{}]--target{}[{}]--error[{}]--data[{}]", client_socket_.get_session_id(), target.serial, target.get_session_id(), ec.message(), bcd_to_hex(reinterpret_cast<const unsigned char*>(&target.buffer), len));
    }
}

void tcp_switch_session::send_to_target(std::size_t len)
{
    for (auto& target : target_socket_list_)
    {
        boost::system::error_code ec;
        std::size_t write_len = boost::asio::write(target.socket, boost::asio::buffer(client_socket_.buffer, len), ec);
        if (!ec)
        {
            LOG_ALL("向target{}写数据成功:client[{}]--target{}[{}]--len[{}]--data[{}]", target.serial, client_socket_.get_session_id(), target.serial, target.get_session_id(), write_len, bcd_to_hex(reinterpret_cast<const unsigned char*>(&client_socket_.buffer), write_len));
            singletion<tcp_flow_statistics>::instance().increment_packet(SEND_TO_TARGET + std::to_string(target.serial));
        }
        else
        {
            LOG_ALL("向target{}写数据失败:client[{}]--target{}[{}]--error[{}]--data[{}]", target.serial, client_socket_.get_session_id(), target.serial, target.get_session_id(), ec.message(), bcd_to_hex(reinterpret_cast<const unsigned char*>(&client_socket_.buffer), len));
        }
    }
}

void tcp_switch_session::reset_keepalive_timer()
{
    try
    {
        auto self(shared_from_this());
        keepalive_timer_.expires_from_now(boost::posix_time::seconds(KEEPALIVE_TIMEOUT_SECONDS));
        keepalive_timer_.async_wait([this, self](const boost::system::error_code& ec)
        {
            if (!closed_ && !ec)
            {
                LOG_ALL("{}分钟都没有接收到client的数据,关闭tcp代理:client[{}]", KEEPALIVE_TIMEOUT_SECONDS / 60, client_socket_.get_session_id());
                close();
            }
        });
    }
    catch (std::exception& e)
    {
        LOG_WARN("捕获到keepalive定时器异常:error[{}]", e.what());
    }
}

void tcp_switch_session::close()
{
    if (!closed_)
    {
        closed_ = true;

        boost::system::error_code ignore_ec;
        keepalive_timer_.cancel(ignore_ec);

        close_target_socket();
        close_client_socket();

        singletion<tcp_flow_statistics>::instance().subtract_connection();
    }
}

void tcp_switch_session::close_target_socket()
{
    for (auto& target : target_socket_list_)
    {
        if (target.socket.is_open())
        {
            boost::system::error_code ignore_ec;
            target.socket.shutdown(boost::asio::socket_base::shutdown_both, ignore_ec);
            target.socket.close(ignore_ec);
        }
    }

    target_socket_list_.clear();
}

void tcp_switch_session::close_client_socket()
{
    if (client_socket_.socket.is_open())
    {
        boost::system::error_code ignore_ec;
        client_socket_.socket.shutdown(boost::asio::socket_base::shutdown_both, ignore_ec);
        client_socket_.socket.close(ignore_ec);
    }
}

