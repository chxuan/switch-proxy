#include "tcp_switch_session_manager.h"
#include "tcp_switch_session.h"
/* #include "public/utility/log_file.h" */

tcp_switch_session_manager::tcp_switch_session_manager(const address& listen_address, const std::vector<address>& target_address_list, int io_threads)
    : io_service_pool_(io_threads),
    acceptor_(io_service_pool_.get_io_service(), boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(listen_address.ip), listen_address.port)),
    target_address_list_(target_address_list)
{

}

void tcp_switch_session_manager::run()
{
    start_accept();

    /* LOG(LOGI_INFO, "TCP转换代理启动成功,正在监听:[%s]", (acceptor_.local_endpoint().address().to_string() + ":" + std::to_string(acceptor_.local_endpoint().port())).c_str()); */

    io_service_pool_.run();
}

void tcp_switch_session_manager::start_accept()
{
    auto switch_session = std::make_shared<tcp_switch_session>(io_service_pool_.get_io_service());
    acceptor_.async_accept(switch_session->get_client_socket().socket, 
                           [this, switch_session](const boost::system::error_code& ec)
    {
        if (!ec)
        {
            switch_session->start(target_address_list_);
        }
        else
        {
            /* LOG(LOGI_WARN, "接收客户端连接失败:message[%s]", ec.message().c_str()); */
        }

        start_accept();
    });
}

