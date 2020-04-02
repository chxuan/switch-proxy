#include "./switch/switch_proxy.h"
#include "./utils/config_file.h"
#include "./utils/singletion.h"
#include "./utils/easylog/easylog.h"

int main(int argc, char* argv[]) 
{
    if (argc != 2)
    {
        std::cout << "无效的参数，正确的程序启动命令为[./switch_proxy_server ./switch_proxy.conf]" << std::endl;
        return -1;
    }

    singletion<config_file>::instance().set_file_path(argv[1]);

    std::string log_path = singletion<config_file>::instance().get_string("log_path");
    int level = singletion<config_file>::instance().get_int("log_level");
    long long log_file_size = singletion<config_file>::instance().get_long_long("log_file_size");

    EASYLOG_INIT(log_path, (log_level)level, log_file_size);

    /* switch_proxy proxy; */
    /* proxy.run(); */

    return 0;
}
