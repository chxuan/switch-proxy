#pragma once
/*
功能:udp socket
日期:2019.11.29
作者:chengxuan
*/
#include <boost/asio.hpp>
#include "../common/address.h"

static const int UDP_BUFFER_SIZE = 4 * 1024;

using UdpBuffer = std::array<char, UDP_BUFFER_SIZE>;

struct udp_socket
{
    udp_socket(boost::asio::io_service& ios) : socket(ios, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)) {}

    udp_socket(boost::asio::io_service& ios, const address& listen_address)
        : socket(ios, boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(listen_address.ip), listen_address.port)) {}

    // 获得远端地址
    std::string get_remote_address()
    {
        return remote_address.address().to_string() + ":" + std::to_string(remote_address.port());
    }

    // 获得本端地址
    std::string get_local_address()
    {
        if (socket.is_open())
        {
            try
            {
                return socket.local_endpoint().address().to_string() + ":" + std::to_string(socket.local_endpoint().port());
            }
            catch (std::exception& e)
            {
            }
        }

        return "";
    }

    boost::asio::ip::udp::socket socket;
    boost::asio::ip::udp::endpoint remote_address;
    UdpBuffer buffer;
};

