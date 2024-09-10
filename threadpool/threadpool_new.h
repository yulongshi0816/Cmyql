//
// Created by 施玉龙 on 2024/8/22.
//

#ifndef THREADPOOL_THREADPOOL_NEW_H
#define THREADPOOL_THREADPOOL_NEW_H

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
#include <future>

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

// 线程池类型
class ThreadPool{
public:
    ThreadPool();
    ~ThreadPool();
    void start(int initThreadSize = 4); //开启线程池
    void setMode(PoolMode mode); // 设置线程池模式
    void setThreadSizeThreshHold(int threshold);
    void setTaskQueueMaxThreadHold(int threadhold); // 设置task任务队列上限的阈值
    // 使用可变参模板编程，提交任务
    template<typename Func, typename... Args>
    auto submitTask(Func&& func, Args&&... args)-> std::future<decltype(func(args...))> {
        // 打包任务，放在任务队列中
        using RType = decltype(func(args...));
        auto task = std::make_shared<std::packaged_task<RType()>>(
                std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        std::future<RType> result = task->get_future();
        std::unique_lock<std::mutex> lock(taskQueMtx_);
        std::cout << "tid: " << std::this_thread::get_id() << "尝试获取线程任务..." << std::endl;
        // 线程通信， 等待任务放在队列中 最长不能超过1s否则提交失败
        if(!notFull_.wait_for(lock, std::chrono::seconds(1), [&]()->bool
        {return taskQue_.size() < taskQueMaxThreadHold_;}))
        {
            //表示超过1s，条件依然没满足
            std::cerr << "task queue is full ,submit task failed." << std::endl;
            auto task = std::make_shared<std::packaged_task<RType()>>(
                    []()->RType {return RType();}
                    );
            (*task)();
            return task->get_future();
        }
        std::cout << "tid: " << std::this_thread::get_id() << "获取任务成功..." << std::endl;
        // 如果空余，任务放入队列中
        taskQue_.emplace([task](){
            // 执行下面的函数
            (*task)();
        });
        taskSize_++;
        // 因为新放任务， notEmpty_肯定不空
        noEmpty_.notify_all();
        // cached模式 任务处理比较紧急，场景：小儿快的任务 需要根据任务数量和空闲线程的数量，判断是否要创建线程；
        if(mode_ == PoolMode::MODE_CACHD && taskSize_ > idleThreadSize_  && curThreadSize_ < threadSizeThreshHold_){
            std::cout<< "create new thread.. " << std::endl;
            // 创建新线程
            auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
            auto threadId = ptr->getId();
            threads_.emplace(threadId, std::move(ptr));
            threads_[threadId]->start();
            curThreadSize_++;
            idleThreadSize_++;
        }
        return result;
    }
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
    using Task = std::function<void()>;
    std::queue<Task> taskQue_; // 任务队列
    std::atomic_uint taskSize_; // 任务数量
    int taskQueMaxThreadHold_;// 任务队列上限阈值
    std::mutex taskQueMtx_; // 保证任务队列的线程安全
    std::condition_variable notFull_; // 任务队列不满
    std::condition_variable noEmpty_; // 任务队列不空
    std::condition_variable exitCond_; // 等待线程资源全部回收
    PoolMode mode_;
    std::atomic_bool isPoolRunning_; //表示线程池的启动状态；
    std::atomic_int idleThreadSize_; //空闲线程的数量

};
#endif //THREADPOOL_THREADPOOL_NEW_H
