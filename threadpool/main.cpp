#include <iostream>
#include "threadpool.h"
#include <chrono>

/*
 * 多线程程序IO密集型和CPU密集型
 * cpu密集型：程序中的指令都是做计算用的
 * IO密集型：程序中指令的执行涉及io操作，比如设备、网络、文件操作
 * 对于多核环境而言，都是可以的；单核情况下，由于cpu密集型的多线程需要切换上下文，消耗较大，因此不适合；
 * 线程的创建和销毁都是非常耗资源的；线程栈本身占用大量内存；线程上下文切换要占用大量时间；大量线程同时唤醒会是系统经常出现锯齿状负载或者
 * 瞬间负载很大导致宕机；
 * IO复用 + 多线程 一般根据cpu的核来确定线程数
 * */

// 有些场景需要获取到线程的返回值
// 计算数字和
// c++17 Any 类型
class MyTask : public Task{
public:
    MyTask(int begin, int end): begin_(begin), end_(end) {

    }
    Any run() override{
        std::cout << "tid: " << std::this_thread::get_id() << "start this task..." << std::endl;
        int sum = 0;
        for(int i = begin_; i <=end_; ++i) {
            sum += i;
        }
        std::cout << "tid: " << std::this_thread::get_id() << "end this task..." << std::endl;
        return sum;
    }
private:
    int begin_;
    int end_;
};
int main() {
    ThreadPool pool;
    pool.start(4);
    Result res = pool.subMitTask(std::make_shared<MyTask>());
    int sum = res.get().cast_<int>();
    pool.subMitTask(std::make_shared<MyTask>());
    pool.subMitTask(std::make_shared<MyTask>());
    pool.subMitTask(std::make_shared<MyTask>());

    getchar();
    return 0;
}