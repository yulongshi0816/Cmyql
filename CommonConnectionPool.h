//
// Created by 施玉龙 on 2024/8/8.
//

#ifndef MYSQLLEARN_COMMONCONNECTIONPOOL_H
#define MYSQLLEARN_COMMONCONNECTIONPOOL_H
#include <string>
#include <queue>
#include <mutex>
#include "Connection.h"
#include <thread>
#include <memory>
#include <condition_variable>
#include <functional>
using namespace std;

class ConnectionPool{
public:
    static  ConnectionPool* getConnectionPool();
    // 给外部提供接口，从连接池中获取一个可用的空闲连接
    shared_ptr<Connection> getConnection();
private:
    ConnectionPool();
    bool loadConfigFile();
    void produceConnectionTask(); // 运行在独立的线程中，负责生产新连接
    void scannerConnectionTask();
    string _ip;
    int _port;
    string _username;
    string _passwd;
    string _dbname;
    int _initSize; // 连接池的出事连接
    int _maxSize; // 连接池的最大连接
    int _maxIdleTime; // 连接池最大空闲事件
    int _connectionTimeout; // 连接池超时时间
    atomic_int _connectionCnt; // 记录连接所创建的connetion的总数量

    queue<Connection*> _connetionQueue; // 存储mysql连接的队列
    mutex _queueMutex; // 维护线程队列的互斥锁
    condition_variable cv; // 设置条件变量用于连接生产和连接消费线程的通信
};
#endif //MYSQLLEARN_COMMONCONNECTIONPOOL_H
