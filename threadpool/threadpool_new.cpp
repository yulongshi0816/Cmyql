//
// Created by 施玉龙 on 2024/8/22.
//

#include "threadpool_new.h"

const int TASK_MAX_THRESHHOLD = 1024;
const int THREAD_MAX_THRESHHOLD = 10;
const int THREAD_MAX_IDLE_TIME = 60;

ThreadPool::ThreadPool():
initThreadSize_(0),
taskSize_(0),
taskQueMaxThreadHold_(TASK_MAX_THRESHHOLD),
mode_(PoolMode::MODE_FIXD),
isPoolRunning_(false),
idleThreadSize_(0),
threadSizeThreshHold_(THREAD_MAX_THRESHHOLD),
curThreadSize_(0)
{

}

ThreadPool::~ThreadPool() {
    isPoolRunning_ = false;
    // 等待线程池中所有线程返回， 线程两种状态： 阻塞 & 正在执行中
    std::unique_lock<std::mutex> lock(taskQueMtx_);
    noEmpty_.notify_all();
    exitCond_.wait(lock, [&](){return threads_.empty();});

}

void ThreadPool::setMode(PoolMode mode) {
    if(checkRunningStat())
        return;
    mode_ = mode;
}

void ThreadPool::setTaskQueueMaxThreadHold(int threadhold) {
    if(checkRunningStat())
        return;
    taskQueMaxThreadHold_ = threadhold;
}

void ThreadPool::setThreadSizeThreshHold(int threshold) {
    if(isPoolRunning_)
        return;
    if(mode_ == PoolMode::MODE_CACHD)
        threadSizeThreshHold_ = threshold;
}

bool ThreadPool::checkRunningStat() const {
    return isPoolRunning_;
}

void ThreadPool::start(int initThreadSize) {
    isPoolRunning_ = true;
    // 记录初始线程个数和数量
    initThreadSize_ = initThreadSize;
    curThreadSize_ = initThreadSize;
    // 创建线程对象
    for(int i = 0; i<initThreadSize_; i++) {
        // 创建线程对象的时候，把线程函数给到thread线程对象
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        auto id = ptr->getId();
        threads_.emplace(id, std::move(ptr));
    }
    // 启动所有线程
    for(int i = 0; i<initThreadSize_; i++) {
        threads_[i]->start();
        idleThreadSize_++;
    }
}

// 定义线程函数， 消费任务
void ThreadPool::threadFunc(int threadid) {
    auto lastTime = std::chrono::high_resolution_clock::now();
    for(;;) {
        Task task;
        {
        // 先获取锁
        std::unique_lock<std::mutex> lock(taskQueMtx_);
        // 超过initThreadSIze的线程要进行回收
        // 当前时间，上一次线程执行的时间 > 60
            // 每一秒返回一次
        while(taskQue_.empty()) {
            if(!isPoolRunning_) {
                threads_.erase(threadid);
                std::cout << "333333 id: " << std::this_thread::get_id() << " exit";
                exitCond_.notify_all();
                return;
            }
            if (mode_ == PoolMode::MODE_CACHD) {
                if (std::cv_status::timeout == noEmpty_.wait_for(lock, std::chrono::seconds(1))) {
                    // 条件变量超时返回
                    auto now = std::chrono::high_resolution_clock::now();
                    auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                    if (dur.count() >= THREAD_MAX_IDLE_TIME && curThreadSize_ > initThreadSize_) {
                        // 开始回收当前线程
                        // 记录线程数量的值的修改
                        // 线程对象要从线程列表容器中删除, 没有办法匹配对应的是哪个线程对象
                        threads_.erase(threadid);
                        // thread_id => thread对象
                        curThreadSize_--;
                        idleThreadSize_--;
                        std::cout << "111111111thread id: " << std::this_thread::get_id() << " exit";
                        return;
                    }
                }
            } else {
                noEmpty_.wait(lock);
            }
        }
        idleThreadSize_--;
        // 取一个任务出来
        task = taskQue_.front();
        taskQue_.pop();
        taskSize_--;
        // 如果有其他任务，继续通知其他线程执行任务
        if(!taskQue_.empty()){
            noEmpty_.notify_all();
        }
        // 取出任务，进行通知
        notFull_.notify_all();
        }
        // 执行这个任务
        if(task != nullptr){
           task();
        }
        idleThreadSize_++;
        lastTime = std::chrono::high_resolution_clock::now(); //更新执行完任务的时间
    }
}

////线程方法的实现
//// 启动线程
int Thread::generateID_ = 0;
void Thread::start() {
    // 创建一个线程来执行一个线程函数
    std::thread t(func_, this->threadID_); // 线程对象t，和线程函数func_
    t.detach(); // 设置分离线程
}

Thread::Thread(ThreadFunc func): func_(func), threadID_(generateID_++) {

}

Thread::~Thread() {

}

int Thread::getId() const {
    return threadID_;
}


























