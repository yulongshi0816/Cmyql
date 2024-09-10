#include <iostream>
#include "threadpool_new.h"
#include <chrono>
#include <future>

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
#ifdef s
class MyTask : public Task{
public:
    MyTask(int begin, int end): begin_(begin), end_(end) {

    }
    Any run() {
        std::cout << "tid: " << std::this_thread::get_id() << "start this task..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
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
    // threadpool西沟后，回收线程资源；
    ThreadPool pool;
    pool.setMode(PoolMode::MODE_CACHD);
    // 用户设置线程池方法
    pool.start(4);
    Result res1 = pool.subMitTask(std::make_shared<MyTask>(1, 100));
    Result res2 = pool.subMitTask(std::make_shared<MyTask>(1, 1000));
    Result res3 = pool.subMitTask(std::make_shared<MyTask>(1, 100));
    // Result对象也被析构了, 在windows，条件变量析构会释放资源

    return 0;
}
#endif
/*
 * submit实现可变参数编程
 * thread库提供了 packaged_task(function函数对象) async
 *
 *
 * */
int sum1(int a, int b){
    return a+b;
}
int main(){
    ThreadPool pool;
    pool.start(4);
    std::future<int> res = pool.submitTask(sum1, 1, 2);
    std::cout << "ans is: " << res.get()<< std::endl;

    return 0;
}