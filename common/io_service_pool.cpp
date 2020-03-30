#include "io_service_pool.h"
#include <boost/bind.hpp>
#include <thread>

io_service_pool::io_service_pool(int pool_size)
{
    for (int i = 0; i < pool_size; ++i)
    {
        auto ios = std::make_shared<boost::asio::io_service>();
        auto work = std::make_shared<boost::asio::io_service::work>(*ios);
        ios_pool_.push_back(ios);
        work_pool_.push_back(work);
    }
}

void io_service_pool::run()
{
    std::vector<std::shared_ptr<std::thread>> threads;

    for (std::size_t i = 0; i < ios_pool_.size(); ++i)
    {
        auto t = std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, ios_pool_[i]));
        threads.push_back(t);
    }

    for (auto& t : threads)
    {
        t->join();
    }
}

void io_service_pool::stop()
{
    for (auto& ios : ios_pool_)
    {
        ios->stop();
    }
}

boost::asio::io_service& io_service_pool::get_io_service()
{
    boost::asio::io_service& ios = *ios_pool_[next_io_service_];
    ++next_io_service_;
    if (next_io_service_ == ios_pool_.size())
    {
        next_io_service_ = 0;
    }

    return ios;
}
