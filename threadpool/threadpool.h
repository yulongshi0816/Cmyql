//
// Created by 施玉龙 on 2024/8/22.
//

#ifndef THREADPOOL_THREADPOOL_H
#define THREADPOOL_THREADPOOL_H

#include <vector>
#include <cstddef>
#include <queue>
#include <memory>
#include <stdatomic.h>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <unordered_map>

static std::mutex *mutex = new std::mutex;

// Any类型，可以接受任一类型的数据
class Any {
public:
    Any() = default;
    ~Any() = default;
    Any(const Any&) = delete;
    Any& operator=(const Any&) = delete;
    Any(Any&&) = default;
    Any& operator=(Any&&) = default;
    // 这个构造函数可以让Any接收任一其他的数据
    template<typename T>
    Any(T data) : base_(std::make_unique<Derive<T>>(data)){

    }
    // 这个方法用来提取数据
    template<typename T> // 如果返回的是int
    T cast_(){
        // 我们整么从base中，找到他所指向的派生类对象中的data
        // 基类指针-》派生类指针
        Derive<T> *pd = dynamic_cast<Derive<T>*>(base_.get());
        if(pd == nullptr){
            throw "type is unmatch";
        }
        return pd->data_;
    }
private:
    // 基类
    class Base{
    public:
    virtual ~Base() = default;
    };
    // 派生类类型
    template<typename T>
    class Derive: public Base{
    public:
        Derive(T data): data_(data){}
        T data_;
    };
private:
    std::unique_ptr<Base> base_;
};

// 实现信号量类
class Semaphore{
public:
    Semaphore(int limit = 0):resLimit_(limit){

    }
    ~Semaphore() = default;
    // 获取信号量资源
    void wait(){
        std::unique_lock<std::mutex> lock(mtx_);
        // 等待信号量
        cond_.wait(lock, [&](){return resLimit_ > 0;});
        resLimit_--;
    }
    // 增加一个信号量资源
    void post(){
        std::unique_lock<std::mutex> lock(mtx_);
        resLimit_++;
        cond_.notify_all();
    }
private:
    std::mutex mtx_;
    int resLimit_;
    std::condition_variable cond_;
};
class Task;
// 实现接收到提交到线程池的task任务执行完成后的返回值类型Result
class Result{
public:
    Result(std::shared_ptr<Task> task, bool isVaild = true);
    ~Result() = default;
    // setVal方法获取任务执行完的返回值
    void setVal(Any any);
    // get方法用户调用获取task的返回值
    Any get();
private:
    Any any_; // 存储任务的返回值；
    Semaphore sem_; // 线程通信的信号量
    std::shared_ptr<Task> task_; // 指向对应获取返回值的任务对象；
    std::atomic_bool isValid_; // 返回值是否有效
};
// 任务抽象基类
class Task {
public:
    Task():result_(nullptr){}
    void exec();
    void setResult(Result* res);
    // 用户可以自定义任一任务类型，从Task继承，重写run方法
    virtual Any run() = 0;
private:
    Result *result_;
};
//线程池支持的模式
enum class PoolMode{
    MODE_FIXD,// 固定数量的线程
    MODE_CACHD, // 线程数量课动态增长
};

// 线程类型
class Thread{
public:
    // 线程函数对象类型
    using ThreadFunc = std::function<void(int)>;
    Thread(ThreadFunc func);
    ~Thread();
public:
    void start();
    int getId() const;
private:
    ThreadFunc func_;
    static int generateID_;
    int threadID_; // 保存线程id
};
/*ThreadPool pool;
 * pool.start(4);
 * class MyTask: public Task{
 * void run();
 * }
 * pool.submitTask(std::make_shared<MyTask>());
 * */

// 线程池类型
class ThreadPool{
public:
    ThreadPool();
    ~ThreadPool();
    void start(int initThreadSize = 4); //开启线程池
    void setMode(PoolMode mode); // 设置线程池模式
    void setThreadSizeThreshHold(int threshold);
    void setTaskQueueMaxThreadHold(int threadhold); // 设置task任务队列上限的阈值
    Result subMitTask(const std::shared_ptr<Task>& sp); // 提交任务
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
private:
    //定义线程函数
    void threadFunc(int threadid);
    // 检查pool的运行状态
    bool checkRunningStat() const;
private:
    //std::vector<std::unique_ptr<Thread>> threads_; // 线程列表
    std::unordered_map<int, std::unique_ptr<Thread>> threads_; // 线程列表
    int initThreadSize_; //初始线程数量
    int threadSizeThreshHold_; // 线程数量上限的阈值
    std::atomic_int curThreadSize_; //     记录当前的线程总数量
    std::queue<std::shared_ptr<Task>> taskQue_; // 任务队列
    std::atomic_uint taskSize_; // 任务数量
    int taskQueMaxThreadHold_;// 任务队列上限阈值
    std::mutex taskQueMtx_; // 保证任务队列的线程安全
    std::condition_variable notFull_; // 任务队列不满
    std::condition_variable noEmpty_; // 任务队列不空
    PoolMode mode_;
    std::atomic_bool isPoolRunning_; //表示线程池的启动状态；
    std::atomic_int idleThreadSize_; //空闲线程的数量

};
#endif //THREADPOOL_THREADPOOL_H
