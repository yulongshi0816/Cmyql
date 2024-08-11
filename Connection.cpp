//
// Created by 施玉龙 on 2024/8/8.
//
#include "public.h"
#include "Connection.h"
#include <iostream>
using namespace std;

Connection::Connection() {
    _conn = mysql_init(nullptr);
}

Connection::~Connection(){
    if(_conn != nullptr)
        mysql_close(_conn);
}

bool Connection::connect(std::string ip, std::string username, std::string passwd, std::string dbname, int port) {
    MYSQL *p = mysql_real_connect(_conn,
                                  ip.c_str(),//主机
                                  username.c_str(),//用户名
                                  passwd.c_str(),//密码
                                  dbname.c_str(),//数据库名 必须是已经存在的 我的mysql大小写不敏感
                                  port,//端口号 默认的0或者3306
                                  nullptr, 0);//最后两个参数的常用写法 几乎都是这两个
    return p != nullptr;
}

bool Connection::update(std::string sql) {
    if (mysql_query(_conn, sql.c_str())) {
        LOG("更新失败" + sql);
        return false;
    }

    return true;
}

MYSQL_RES* Connection::query(std::string sql) {
    if(mysql_query(_conn, sql.c_str())) {
        LOG("更新失败" + sql);
        return nullptr;
    }

    return mysql_use_result(_conn);
}