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
    clock_t begin = clock();
#if 0
    for(int i = 0; i< 1000; ++i) {
        char sql[1024] = {0};
        sprintf(sql, "insert into user(name, age, sex) value('%s', '%d', '%s')", "zhangsan", 20, "male");
        shared_ptr<Connection> sp = cp->getConnection();
        sp->update(sql);
    }

#endif
    thread t1([](){
        ConnectionPool *cp = ConnectionPool::getConnectionPool();
        for(int i = 0; i< 250; ++i) {
            char sql[1024] = {0};
            sprintf(sql, "insert into user(name, age, sex) value('%s', '%d', '%s')", "zhangsan", 20, "male");
            shared_ptr<Connection> sp = cp->getConnection();
            sp->update(sql);
        }
    });

    thread t2([](){
        ConnectionPool *cp = ConnectionPool::getConnectionPool();
        for(int i = 0; i< 250; ++i) {
            char sql[1024] = {0};
            sprintf(sql, "insert into user(name, age, sex) value('%s', '%d', '%s')", "zhangsan", 20, "male");
            shared_ptr<Connection> sp = cp->getConnection();
            sp->update(sql);
        }
    });
    t1.join();
    t2.join();
    clock_t end= clock();
    cout << (end - begin) << "ms" << endl;
    return 0;
}
