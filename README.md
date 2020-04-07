## switch-proxy

> switch-proxy是采用C++开发基于Boost.Asio的网络转发代理服务。

## 编译

### 添加boost依赖环境变量

编辑`~/.bashrc`，添加如下内容
```
export BOOST_INCLUDE_PATH="your/boost/include/path"
export BOOST_LIB_PATH="your/boost/lib/path"

```
然后执行`source ~/.bashrc`让配置生效

### 编译switch-proxy

    cd ./switch-proxy/build
    cmake ..
    make

## 运行

    ./switch_proxy_server ./switch_proxy.conf

控制台或日志显示如下信息则表示服务启动成功

    I2020-04-07 01:41:28.710 tcp_switch_session_manager.cpp:17] TCP转发代理启动成功,正在监听:[0.0.0.0:8888]

## 配置文件

```
#服务监听地址
listen_ip=0.0.0.0
listen_port=8888

#io线程数量
io_threads=4

#日志级别[all = 0, trace, debug, info, warn, error, fatal]
log_level=0

#日志路径
log_path=./log

#日志文件大小[100*1024*1024=104857600=100MB]
log_file_size=104857600

#协议类型[tcp或udp]
type=udp

#转发的目标服务地址,编号[0~99]
target_ip0=127.0.0.1
target_port0=9999

target_ip1=127.0.0.1
target_port1=10000

```
