#pragma once
/*
功能:tcp socket
日期:2020-04-02
作者:chxuan <787280310@qq.com>
*/
#include <boost/asio.hpp>
#include "../common/address.h"

static const int TCP_BUFFER_SIZE = 1460;// MTU=MSS+TCP Header+IP Header. 1500=1460+20+20

using TcpBuffer = std::array<char, TCP_BUFFER_SIZE>;

struct tcp_socket
{
    tcp_socket(boost::asio::io_service& ios) : socket(ios) {}

    tcp_socket(boost::asio::io_service& ios, int serial, const address& remote_address) 
        : socket(ios), serial(serial), remote_address(remote_address) {}

    // 获得会话id
    std::string get_session_id()
    {
        return get_local_address() + "_" + get_remote_address();
    }

    // 获得远端地址
    std::string get_remote_address()
    {
        if (socket.is_open())
        {
            try
            {
                return socket.remote_endpoint().address().to_string() + ":" + std::to_string(socket.remote_endpoint().port());
            }
            catch (std::exception& e)
            {
            }
        }

        return "";
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

    boost::asio::ip::tcp::socket socket;
    int serial = -1;
    address remote_address { "", 0 };
    TcpBuffer buffer;
};

