# *advanced computer network* coursework

1. Client/Server双向收发消息
    - 功能：
        - 客户端与服务器程序双向收发消息
    - 运行方法：
        1. 编译：进入该目录，执行`make cs`，即可生成server和client可执行文件
        2. 运行服务端程序：`./server port`,port为监听的端口号
        3. 运行客户端程序：打开另一个终端执行`./client port`，port为服务端程序监听的端口号。

2. 简单HTTP服务器
    - 功能：
        - 简易的HTTP服务器，可在参数中指定站点根目录。
    - 运行方法：
        1. 编译：进入该目录，执行`make httpserver`，即可生成httpserver可执行文件
        2. 运行服务端程序：`./main [-p 80][-m 6][-d .]`,其中：
            - -p: port
            - -m: thread_max_num
            - -d: rootdir

- 如果同时编译两个程序，可直接执行`make`或`make all`
- 若要清除编译生成的可执行文件和中间文件，执行`make clean`
