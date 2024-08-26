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
    Any(T data) :base_(std::make_shared<Derive<T>>(data)){

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
// 任务抽象基类
class Task {
public:
    // 用户可以自定义任一任务类型，从Task继承，重写run方法
    virtual Any run() = 0;
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
    using ThreadFunc = std::function<void()>;
    Thread(ThreadFunc func);
    ~Thread();
public:
    void start();
private:
    ThreadFunc func_;
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
    void setTaskQueueMaxThreadHold(int threadhold); // 设置task任务队列上限的阈值
    void subMitTask(const std::shared_ptr<Task>& sp); // 提交任务
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
private:
    //定义线程函数
    void threadFunc();
private:
    std::vector<std::unique_ptr<Thread>> threads_; // 线程列表
    int initThreadSize_; //初始线程数量
    std::queue<std::shared_ptr<Task>> taskQue_; // 任务队列
    std::atomic_uint taskSize_; // 任务数量
    int taskQueMaxThreadHold_;// 任务队列上限阈值
    std::mutex taskQueMtx_; // 保证任务队列的线程安全
    std::condition_variable notFull_; // 任务队列不满
    std::condition_variable noEmpty_; // 任务队列不空
    PoolMode mode_;

};
#endif //THREADPOOL_THREADPOOL_H
