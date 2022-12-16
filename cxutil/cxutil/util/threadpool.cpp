#include "threadpool.h"
#include <iostream>
#include "ccglobal/log.h"

using namespace std;

namespace cxutil
{
    void CXThreadPool::init(int num)
    {
        unique_lock<mutex> lock(mux_);
        this->thread_num_ = num;
        LOGI("Thread pool Init %d ",num);
    }
    void CXThreadPool::start()
    {
        unique_lock<mutex> lock(mux_);
        if (thread_num_ <= 0)
        {
            LOGE("Please Init XThreadPool");
            return;
        }
        if (!threads_.empty())
        {
            //cerr << "Thread pool has start!" << endl;
            LOGE("Thread pool has start!");
            return;
        }
        //Æô¶¯Ïß³Ì
        for (int i = 0; i < thread_num_; i++)
        {
            //auto th = new thread(&XThreadPool::Run, this);
            auto th = make_shared<thread>(&CXThreadPool::run, this);
            threads_.push_back(th);
        }
    }
    void CXThreadPool::stop()
    {
        is_exit_ = true;
        cv_.notify_all();
        for (auto& th : threads_)
        {
            th->join();
        }
        unique_lock<mutex> lock(mux_);
        threads_.clear();
    }
    void CXThreadPool::run()
    {
        //cout << "begin XThreadPool Run " << this_thread::get_id() << endl;
        LOGI("begin XThreadPool Run %d ", this_thread::get_id());
        while (!is_exit())
        {
            auto task = getTask();
            if (!task)continue;
            ++task_run_count_;
            try
            {
                auto re = task->run();
                //task->setValue(re);
            }
            catch (...)
            {

            }
            --task_run_count_;
        }

       // cout << "end XThreadPool Run " << this_thread::get_id() << endl;
        LOGI("end XThreadPool Run %d ", this_thread::get_id());
    }
    void CXThreadPool::addTask(std::shared_ptr<CXTask> task)
    {
        unique_lock<mutex> lock(mux_);
        tasks_.push_back(task);
        task->is_exit = [this] {return is_exit(); };
        lock.unlock();
        cv_.notify_one();
    }

    std::shared_ptr<CXTask> CXThreadPool::getTask()
    {
        unique_lock<mutex> lock(mux_);
        if (tasks_.empty())
        {
            cv_.wait(lock);
        }
        if (is_exit())
            return nullptr;
        if (tasks_.empty())
            return nullptr;
        auto task = tasks_.front();
        tasks_.pop_front();
        return task;
    }
}
