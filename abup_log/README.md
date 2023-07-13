# log

远程诊断log模块，主要实现log日志打印，日志收集等功能。



## 代码目录结构

    log
    └──inc
        └──  abup_log_core.h        对外函数声明文件      
        └──  abup_log_zlog.h        对外数据结构声明文件
    └──src 
        └──  abup_log_core.c        对外函数实现文件
        └──  abup_log_zlog.c        对外数据结构实现文件
    └──example
        └──  log_demo.c             log模块测试demo



    log模块初始化后，存储log模块配置文件及真实log文件目录

    log
    └──abup_log_conf       log模块配置文件
    └──logs                log日志具体存放的目录
       └──uc.log          uc log存放的具体文件
       └──vdi.log         vdi log存放的具体文件