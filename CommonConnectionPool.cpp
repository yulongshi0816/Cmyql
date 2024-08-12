//
// Created by 施玉龙 on 2024/8/8.
//
#include <iostream>
#include "CommonConnectionPool.h"
using namespace std;
#include "public.h"


// 线程安全的懒汉单力模式
ConnectionPool* ConnectionPool::getConnectionPool() {
    static ConnectionPool pool; // 静态变量，由编译器自动lock和ulock
    return &pool;
}

bool ConnectionPool::loadConfigFile() {
    FILE *pf = fopen("../mysql.cnf", "r");
    if(pf == nullptr) {
        LOG("mysql.inim error");
        printf("Error: %s\n", strerror(errno));
        return false;
    }

    while (!feof(pf)) {
        char line[1024] = {0};
        fgets(line, 1024, pf);
        string str = line;
        int idx = str.find('=', 0);
        if(idx == -1) {
            continue;
        }

        int endidx = str.find('\n', idx);
        string key = str.substr(0, idx);
        string value = str.substr(idx+1, endidx-idx-1);
        if(key == "ip") {
            _ip = value;
        }
        else if(key == "port") {
            _port = atoi(value.c_str());
        }
        else if( key == "username") {
            _username = value;
        }
        else if(key == "dbname") {
            _dbname = value;
        }
        else if(key == "passwd") {
            _passwd = value;
        }
        else if(key == "initSize") {
            _initSize = atoi(value.c_str());
        }
        else if(key == "maxSize") {
            _maxSize = atoi(value.c_str());
        }
        else if(key == "maxIdleTime") {
            _maxIdleTime = atoi(value.c_str());
        }
        else if(key == "maxConnectionTimeOut") {
            _connectionTimeout = atoi(value.c_str());
        }
    }
    return true;
}

ConnectionPool::ConnectionPool() {
    // 加载配置项
    if(!loadConfigFile()) {
        return;
    }

    // 创建初施数量的连接
    for (int i = 0; i < _initSize; i++) {
        Connection* p = new Connection();
        p->connect(_ip, _username, _passwd, _dbname, _port);
        p->refreshAliveTime();
        _connetionQueue.push(p);
        _connectionCnt++;
    }
    // 启动一个新的线程做为连接的生产者
    thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
    produce.detach();
    // 启动一个新的定时线程，扫描多余的空闲连接，超过maxIdleTime的空闲连接，进行连接回收。
    thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach();
}

void ConnectionPool::produceConnectionTask() {
    for(;;) {
        unique_lock<mutex> lock(_queueMutex);
        while(!_connetionQueue.empty()) {
            cv.wait(lock); // 队列不空，进入等待状态
        }

        // 连接数量没有到达上限，继续创建新的连接
        if(_connectionCnt < _maxSize) {
            Connection* p = new Connection();
            p->connect(_ip, _username, _passwd, _dbname, _port);
            p->refreshAliveTime(); //  刷新开始空闲的起始时间
            _connetionQueue.push(p);
            _connectionCnt++;
        }
        cv.notify_all();
    }
}

shared_ptr<Connection> ConnectionPool::getConnection() {
    unique_lock<mutex> lock(_queueMutex);
    while (_connetionQueue.empty()) {
        if(cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout))){
            if(_connetionQueue.empty()) {
                LOG("get time serr");
                return nullptr;
            }
        }
    }
    // share_ptr析构时，会把conntion的资源直接delte掉，因此需要自定义share——ptr释放资源的方式
    shared_ptr<Connection> sp(_connetionQueue.front(), [&](Connection* pcon){
        // 服务器应用线程中调用，需要考虑线程安全
        unique_lock<mutex> lock(_queueMutex);
        pcon->refreshAliveTime();
        _connetionQueue.push(pcon);
    } );
    _connetionQueue.pop();
    cv.notify_all(); // 消费完成后，通知其他线程消费
    return sp;
}

void ConnectionPool::scannerConnectionTask() {
    for(;;) {
        // 模拟定时效果
        this_thread::sleep_for(chrono::seconds(_maxIdleTime));
        // 扫描整个队列，释放多余的连接
        unique_lock<mutex> lock(_queueMutex);
        if(_connectionCnt > _initSize) {
            Connection* p = _connetionQueue.front();
            if(p->getAliveTime() >= (_maxIdleTime * 1000)) {
                _connetionQueue.pop();
                delete p;// 释放连接
                _connectionCnt--;
            }
            else{
                break;
                // 队头的连接没有超过，其他连接肯定没有
            }
        }
    }
}