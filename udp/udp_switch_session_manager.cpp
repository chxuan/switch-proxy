#include "udp_switch_session_manager.h"
#include "udp_switch_session.h"
#include "../utils/udp_flow_statistics.h"
#include "../utils/singletion.h"
#include "../utils/switch_proxy_util.h"

using namespace sp::util;

const static std::string SEND_TO_CLIENT = "send_to_client";
const static std::string RECEIVE_FROM_CLIENT = "receive_from_client";

udp_switch_session_manager::udp_switch_session_manager(const address& listen_address, const std::vector<address>& target_address_list, int io_threads)
    : io_service_pool_(io_threads),
    client_socket_(io_service_pool_.get_io_service(), listen_address),
    target_address_list_(target_address_list)
{
}

void udp_switch_session_manager::run()
{
    async_receive_client();

    LOG_INFO("UDP转换代理启动成功,正在监听:[%s]", client_socket_.get_local_address());

    io_service_pool_.run();
}

void udp_switch_session_manager::send_to_client(udp_socket& target_socket, std::size_t len, const address& client_address)
{
    try
    {
        std::size_t send_len = client_socket_.socket.send_to(boost::asio::buffer(target_socket.buffer, len), 
                                                             boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(client_address.ip), client_address.port));
        LOG_ALL("向client写数据成功:client[{}]--target[{}]--len[{}]--data[{}]", client_address.addr, target_socket.get_remote_address(), send_len, bcd_to_hex(reinterpret_cast<const unsigned char*>(&target_socket.buffer), send_len));
        singletion<udp_flow_statistics>::instance().increment_packet(SEND_TO_CLIENT);
    }
    catch (std::exception& e)
    {
        LOG_ALL("向client写数据失败:client[{}]--target[{}]--error[{}]--data[{}]", client_address.addr, target_socket.get_remote_address(), e.what(), bcd_to_hex(reinterpret_cast<const unsigned char*>(&target_socket.buffer), len));
    }
}

void udp_switch_session_manager::udp_switch_session_closed(const std::string& client_address)
{
    std::lock_guard<std::mutex> lock(mutex_);

    udp_switch_session_cache_.erase(client_address);

    LOG_INFO("当前连接数:count[{}]", udp_switch_session_cache_.size());
}

void udp_switch_session_manager::async_receive_client()
{
    client_socket_.socket.async_receive_from(boost::asio::buffer(client_socket_.buffer), client_socket_.remote_address, 
                                             [this](const boost::system::error_code& ec, std::size_t len)
    {
        if (!ec)
        {
            std::string client_address = client_socket_.get_remote_address();
            auto switch_session = get_or_create_udp_switch_session(client_address);

            LOG_ALL("接收到client的数据:client[{}]--len[{}]--data[{}]", client_address, len, bcd_to_hex(reinterpret_cast<const unsigned char*>(&client_socket_.buffer), len));
            singletion<udp_flow_statistics>::instance().increment_packet(RECEIVE_FROM_CLIENT);

            switch_session->send_to_target(client_socket_.buffer, len);
            async_receive_client();
        }
        else if (ec != boost::asio::error::operation_aborted)
        {
            LOG_WARN("接收client数据失败:client[{}]--error[{}]", client_socket_.get_remote_address(), ec.message());
        }
    });
}

std::shared_ptr<udp_switch_session> udp_switch_session_manager::get_or_create_udp_switch_session(const std::string& client_address)
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::shared_ptr<udp_switch_session> switch_session;

    auto iter = udp_switch_session_cache_.find(client_address);
    if (iter == udp_switch_session_cache_.end())
    {
        switch_session = std::make_shared<udp_switch_session>(io_service_pool_.get_io_service(), target_address_list_, address(client_address), this);
        switch_session->async_receive_target();

        udp_switch_session_cache_.emplace(client_address, switch_session);

        LOG_ALL("接收到客户端连接:client[{}]", client_socket_.get_remote_address());
        LOG_INFO("当前连接数:count[{}]", udp_switch_session_cache_.size());
    }
    else
    {
        switch_session = iter->second;
    }

    return switch_session;
}
