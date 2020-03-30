#include "tcp_switch_session.h"
#include "../utils/tcp_flow_statistics.h"
/* #include "public/utility/singletion_template.h" */
/* #include "public/utility/binary_calculate.h" */
/* #include "public/utility/log_file.h" */

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
    /* LOG(LOGI_ALL, "接收到客户端连接:client[%s]", client_socket_.get_session_id().c_str()); */
    /* singletion<tcp_flow_statistics>::getinstance()->increment_connection(); */

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
            /* LOG(LOGI_ALL, "连接target%d成功:client[%s]--target%d[%s]", target.serial, client_socket_.get_session_id().c_str(), target.serial, target.get_session_id().c_str()); */
        }
        catch (std::exception& e)
        {
            /* LOG(LOGI_WARN, "连接target%d失败:client[%s]--target%d[%s]--error[%s]", target.serial, client_socket_.get_session_id().c_str(), target.serial, target.get_session_id().c_str(), e.what()); */
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

            /* LOG(LOGI_ALL, "接收到client的数据:client[%s]--len[%u]--data[%s]", client_socket_.get_session_id().c_str(), len, bcd_to_hex(reinterpret_cast<const unsigned char*>(&client_socket_.buffer), len).c_str()); */
            /* singletion<tcp_flow_statistics>::getinstance()->increment_packet(RECEIVE_FROM_CLIENT); */

            send_to_target(len);
            async_read_client();
        }
        // boost::asio::error::operation_aborted
        // 正在async_read_some()异步任务等待时，本端关闭套接字
        else if (ec != boost::asio::error::operation_aborted)
        {
            /* LOG(LOGI_WARN, "接收client数据失败,关闭tcp代理:client[%s]--error[%s]", client_socket_.get_session_id().c_str(), ec.message().c_str()); */
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
            /* LOG(LOGI_ALL, "接收到target%d的数据:client[%s]--target%d[%s]--len[%u]--data[%s]", target.serial, client_socket_.get_session_id().c_str(), target.serial, target.get_session_id().c_str(), len, bcd_to_hex(reinterpret_cast<const unsigned char*>(&target.buffer), len).c_str()); */
            /* singletion<tcp_flow_statistics>::getinstance()->increment_packet(RECEIVE_FROM_TARGET + std::to_string(target.serial)); */

            send_to_client(target, len);
            async_read_target(target);
        }
        // boost::asio::error::operation_aborted
        // 正在async_read_some()异步任务等待时，本端关闭套接字
        else if (ec != boost::asio::error::operation_aborted)
        {
            /* LOG(LOGI_WARN, "接收target%d数据失败:client[%s]--target%d[%s]--error[%s]", target.serial, client_socket_.get_session_id().c_str(), target.serial, target.get_session_id().c_str(), ec.message().c_str()); */
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
        /* LOG(LOGI_ALL, "向client写数据成功:client[%s]--target%d[%s]--len[%u]--data[%s]", client_socket_.get_session_id().c_str(), target.serial, target.get_session_id().c_str(), write_len, bcd_to_hex(reinterpret_cast<const unsigned char*>(&target.buffer), write_len).c_str()); */
        /* singletion<tcp_flow_statistics>::getinstance()->increment_packet(SEND_TO_CLIENT); */
    }
    else
    {
        /* LOG(LOGI_ALL, "向client写数据失败:client[%s]--target%d[%s]--error[%s]--data[%s]", client_socket_.get_session_id().c_str(), target.serial, target.get_session_id().c_str(), ec.message().c_str(), bcd_to_hex(reinterpret_cast<const unsigned char*>(&target.buffer), len).c_str()); */
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
            /* LOG(LOGI_ALL, "向target%d写数据成功:client[%s]--target%d[%s]--len[%u]--data[%s]", target.serial, client_socket_.get_session_id().c_str(), target.serial, target.get_session_id().c_str(), write_len, bcd_to_hex(reinterpret_cast<const unsigned char*>(&client_socket_.buffer), write_len).c_str()); */
            /* singletion<tcp_flow_statistics>::getinstance()->increment_packet(SEND_TO_TARGET + std::to_string(target.serial)); */
        }
        else
        {
            /* LOG(LOGI_ALL, "向target%d写数据失败:client[%s]--target%d[%s]--error[%s]--data[%s]", target.serial, client_socket_.get_session_id().c_str(), target.serial, target.get_session_id().c_str(), ec.message().c_str(), bcd_to_hex(reinterpret_cast<const unsigned char*>(&client_socket_.buffer), len).c_str()); */
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
                /* LOG(LOGI_ALL, "%d分钟都没有接收到client的数据,关闭tcp代理:client[%s]", KEEPALIVE_TIMEOUT_SECONDS / 60, client_socket_.get_session_id().c_str()); */
                close();
            }
        });
    }
    catch (std::exception& e)
    {
        /* LOG(LOGI_WARN, "捕获到keepalive定时器异常:error[%s]", e.what()); */
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

        /* singletion<tcp_flow_statistics>::getinstance()->subtract_connection(); */
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

