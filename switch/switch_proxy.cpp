#include "switch_proxy.h"
#include "../utils/config_file.h"
#include "../utils/singletion.h"
#include "../tcp/tcp_switch_session_manager.h"
#include "../udp/udp_switch_session_manager.h"

void switch_proxy::run()
{
    std::string listen_ip = singletion<config_file>::instance().get_string("listen_ip");
    int listen_port = singletion<config_file>::instance().get_int("listen_port");
    address listen_address(listen_ip, listen_port);

    std::vector<address> target_address_list = read_target_address();

    int io_threads = singletion<config_file>::instance().get_int("io_threads");

    std::string type = singletion<config_file>::instance().get_string("type");
    if (type == "tcp")
    {
        tcp_switch_session_manager manager(listen_address, target_address_list, io_threads);
        manager.run();
    }
    else
    {
        udp_switch_session_manager manager(listen_address, target_address_list, io_threads);
        manager.run();
    }
}

std::vector<address> switch_proxy::read_target_address()
{
    const static std::string TARGET_IP_PREFIX = "target_ip";
    const static std::string TARGET_PORT_PREFIX = "target_port";

    std::vector<address> target_address_list;

    for (int i = 0; i <= 99; ++i)
    {
        std::string ip_key = TARGET_IP_PREFIX + std::to_string(i);
        std::string port_key = TARGET_PORT_PREFIX + std::to_string(i);
        if (singletion<config_file>::instance().is_exist(ip_key) && singletion<config_file>::instance().is_exist(port_key))
        {
            std::string ip = singletion<config_file>::instance().get_string(ip_key);
            int port = singletion<config_file>::instance().get_int(port_key);
            target_address_list.emplace_back(address(ip, port));
        }
    }

    return target_address_list;
}

