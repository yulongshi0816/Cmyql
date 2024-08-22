//
// Created by 施玉龙 on 2024/8/22.
//

#include "threadpool.h"

const int TASK_MAX_THRESHHOLD = 1024;

ThreadPool::ThreadPool():
initThreadSize_(0),
taskSize_(0),
taskQueMaxThreadHold_(TASK_MAX_THRESHHOLD),
mode_(PoolMode::MODE_FIXD)
{

}

ThreadPool::~ThreadPool() {

}

void ThreadPool::setMode(PoolMode mode) {
    mode_ = mode;
}

void ThreadPool::setTaskQueueMaxThreadHold(int threadhold) {
    taskQueMaxThreadHold_ = threadhold;
}

// 线程池提交任务，用户提交任务
void ThreadPool::subMitTask(std::shared_ptr<Task> sp) {
    // 获取锁
    std::unique_lock lock(taskQueMtx_);
    // 线程通信， 等待任务放在队列中 最长不能超过1s否则提交失败
    notFull_.wait(lock, [&]()->bool {return taskQue_.size() < taskQueMaxThreadHold_;});
    // 如果空余，任务放入队列中
    taskQue_.emplace(sp);
    taskSize_++;
    // 因为新放任务， notEmpty_肯定不空
    noEmpty_.notify_all();
}

void ThreadPool::start(int initThreadSize) {
    // 记录初始线程个数和数量
    initThreadSize_ = initThreadSize;
    // 创建线程对象
    for(int i = 0; i<initThreadSize_; i++) {
        // 创建线程对象的时候，把线程函数给到thread线程对象
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this));
        threads_.emplace_back(std::move(ptr));
    }
    // 启动所有线程
    for(int i = 0; i<initThreadSize_; i++) {
        threads_[i]->start();
    }
}

// 定义线程函数， 消费任务
void ThreadPool::threadFunc() {
    std::unique_lock lock(taskQueMtx_);
    std::cout << "begin  thread::func tid: " << std::this_thread::get_id() << std::endl;
    std::cout << "end  thread::func tid: " << std::this_thread::get_id() <<std::endl;
}

////线程方法的实现
//// 启动线程
void Thread::start() {
    // 创建一个线程来执行一个线程函数
    std::thread t(func_); // 线程对象t，和线程函数func_
    t.detach(); // 设置分离线程
}

Thread::Thread(ThreadFunc func): func_(func) {

}

Thread::~Thread() {

}

























