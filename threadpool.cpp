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
Result ThreadPool::subMitTask(const std::shared_ptr<Task>& sp) {
    // 获取锁
    std::unique_lock<std::mutex> lock(taskQueMtx_);
    std::cout << "tid: " << std::this_thread::get_id() << "尝试获取线程任务..." << std::endl;
    // 线程通信， 等待任务放在队列中 最长不能超过1s否则提交失败
    if(!notFull_.wait_for(lock, std::chrono::seconds(1), [&]()->bool
                                                                {return taskQue_.size() < taskQueMaxThreadHold_;}))
    {
        //表示超过1s，条件依然没满足
        std::cerr << "task queue is full ,submit task failed." << std::endl;
        return Result(sp, false);
    }
    std::cout << "tid: " << std::this_thread::get_id() << "获取任务成功..." << std::endl;
    // 如果空余，任务放入队列中
    taskQue_.emplace(sp);
    taskSize_++;
    // 因为新放任务， notEmpty_肯定不空
    noEmpty_.notify_all();
    return Result(sp);
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
    for(;;) {
        std::shared_ptr<Task> sp;
        {
        // 先获取锁
        std::unique_lock<std::mutex> lock(taskQueMtx_);
        // 等待notempty
        noEmpty_.wait(lock, [&]() { return !taskQue_.empty(); });
        // 取一个任务出来
        sp = taskQue_.front();
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
        if(sp != nullptr){
            //sp->run(); // 执行任务，将放回值给到Result
            sp->exec();
        }
    }
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

/// task方法实现
void Task::exec() {
    if(result_)
        result_->setVal(run());
}

void Task::setResult(Result *res) {
    result_ = res;
}

/// result方法实现
Result::Result(std::shared_ptr<Task> task, bool isVaild): isValid_(isVaild), task_(task) {
    task_->setResult(this);
}

Any Result::get() {
    if(!isValid_){
        return {};
    }
    sem_.wait(); // task如果没有执行完，这里会阻塞用户线程
    return std::move(any_);
}

void Result::setVal(Any any) {
    // 存储task的返回值
    this->any_ = std::move(any);
    sem_.post(); // 以及ing获取返回值，增加信号量资源
}
























