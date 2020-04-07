#include "udp_switch_session.h"
#include "udp_switch_session_manager.h"
#include "../utils/udp_flow_statistics.h"
#include "../utils/singletion.h"
#include "../utils/switch_proxy_util.h"

using namespace sp::util;

const static std::string SEND_TO_TARGET = "send_to_target";
const static std::string RECEIVE_FROM_TARGET = "receive_from_target";

const static int KEEPALIVE_TIMEOUT_SECONDS = 8 * 60;

udp_switch_session::udp_switch_session(boost::asio::io_service& ios, const std::vector<address>& target_address_list, const address& client_address, udp_switch_session_manager* session_manager)
    : ios_(ios),
    keepalive_timer_(ios),
    target_socket_(ios),
    client_address_(client_address),
    session_manager_(session_manager),
    target_address_list_(target_address_list)
{
}

void udp_switch_session::send_to_target(const UdpBuffer& buffer, std::size_t len)
{
    if (closed_)
    {
        return;
    }

    reset_keepalive_timer();

    for (auto& target : target_address_list_)
    {
        try
        {
            std::size_t send_len = target_socket_.socket.send_to(boost::asio::buffer(buffer, len), boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(target.ip), target.port));
            LOG_ALL("向target写数据成功:client[{}]--target[{}]--len[{}]--data[{}]", client_address_.addr, target.addr, send_len, bcd_to_hex(reinterpret_cast<const unsigned char*>(&buffer), send_len));
            singletion<udp_flow_statistics>::instance().increment_packet(SEND_TO_TARGET + "[" + target.addr + "]");
        }
        catch (std::exception& e)
        {
            LOG_ALL("向target写数据失败:client[{}]--target[{}]--error[{}]--data[{}]", client_address_.addr, target.addr, e.what(), bcd_to_hex(reinterpret_cast<const unsigned char*>(&buffer), len));
        }
    }
}

void udp_switch_session::async_receive_target()
{
    if (closed_)
    {
        return;
    }

    target_socket_.socket.async_receive_from(boost::asio::buffer(target_socket_.buffer), target_socket_.remote_address, 
                                             [this](const boost::system::error_code& ec, std::size_t len)
    {
        if (closed_)
        {
            return;
        }

        if (!ec)
        {
            LOG_ALL("接收到target的数据:client[{}]--target[{}]--len[{}]--data[{}]", client_address_.addr, target_socket_.get_remote_address(), len, bcd_to_hex(reinterpret_cast<const unsigned char*>(&target_socket_.buffer), len));
            singletion<udp_flow_statistics>::instance().increment_packet(RECEIVE_FROM_TARGET + "[" + target_socket_.get_remote_address() + "]");

            session_manager_->send_to_client(target_socket_, len, client_address_);
            async_receive_target();
        }
        // boost::asio::error::operation_aborted
        // 正在async_receive_from()异步任务等待时，本端关闭套接字
        else if (ec != boost::asio::error::operation_aborted)
        {
            LOG_WARN("接收target数据失败:client[{}]--target[{}]--error[{}]", client_address_.addr, target_socket_.get_remote_address(), ec.message());
        }
    });
}

void udp_switch_session::reset_keepalive_timer()
{
    try
    {
        keepalive_timer_.expires_from_now(boost::posix_time::seconds(KEEPALIVE_TIMEOUT_SECONDS));
        keepalive_timer_.async_wait([this](const boost::system::error_code& ec)
        {
            if (!closed_ && !ec)
            {
                LOG_ALL("{}分钟都没有接收到client的数据,关闭udp代理--client[{}]", KEEPALIVE_TIMEOUT_SECONDS / 60, client_address_.addr);
                close();
            }
        });
    }
    catch (std::exception& e)
    {
        LOG_WARN("捕获到keepalive定时器异常:error[{}]", e.what());
    }
}

void udp_switch_session::close()
{
    if (!closed_)
    {
        closed_ = true;

        boost::system::error_code ignore_ec;
        keepalive_timer_.cancel(ignore_ec);

        if (target_socket_.socket.is_open())
        {
            boost::system::error_code ignore_ec;
            target_socket_.socket.shutdown(boost::asio::socket_base::shutdown_both, ignore_ec);
            target_socket_.socket.close(ignore_ec);
        }

        session_manager_->udp_switch_session_closed(client_address_.addr);
    }
}
