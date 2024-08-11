//
// Created by 施玉龙 on 2024/8/8.
//

#ifndef MYSQLLEARN_CONNECTION_H
#define MYSQLLEARN_CONNECTION_H
#include <mysql.h>
#include <string>
#include <ctime>
// 实现mysql的增删改查
class Connection{
public:
    Connection();
    ~Connection();
    bool connect(std::string ip, std::string username, std::string passwd, std::string dbname, int port);
    bool update(std::string sql);
    MYSQL_RES* query(std::string sql);
    void refreshAliveTime() {_alivetime = clock();} // 刷新一下连接的起始的空闲时间点
    // 返回存活时间
    clock_t getAliveTime() const {return clock() - _alivetime;}
private:
    MYSQL *_conn; // 表示和mysql的一条连接
    clock_t  _alivetime; //    记录进入空闲状态后的起始存活时间
};

#endif //MYSQLLEARN_CONNECTION_H
