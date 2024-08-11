#include <iostream>
#include "Connection.h"
#include "CommonConnectionPool.h"

using namespace std;


int main() {
//    Connection conn;
//    char sql[1024] = {0};
//    sprintf(sql, "insert into user(name, age, sex) value('%s', '%d', '%s')", "zhangsan", 20, "male");
//    conn.connect();
//    conn.update(sql);
    ConnectionPool *cp = ConnectionPool::getConnectionPool();
    return 0;
}
