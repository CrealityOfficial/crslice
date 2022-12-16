#ifndef _CX_SLICESERVICE_THREADPOOL_H
#define _CX_SLICESERVICE_THREADPOOL_H
#include <thread>
#include <mutex>
#include <vector>
#include <list>
#include <functional>
#include <atomic>
#include <future>

namespace cxutil
{
    class TaskResult;
    class CXTask
    {
    public:
        std::string taskName;
        virtual int run() = 0;
        std::function<bool()> is_exit = nullptr;
        cxutil::TaskResult* getReturn()
        {
            return p_.get_future().get();
        }
        void setValue(TaskResult* result)
        {
            p_.set_value(result);
        }
    private:
        std::promise<TaskResult*> p_;
    };

    class CXThreadPool
    {
    public:
        //////////////////////////////////////////////
        /// 初始化线程池
        /// @para num 线程数量
        void init(int num);

        //////////////////////////////////////////////
        /// 启动所有线程，必须先调用Init
        void start();

        //////////////////////////////////////////////
        /// 线程池退出
        void stop();

        //void AddTask(XTask* task);
        void addTask(std::shared_ptr<CXTask> task);

        std::shared_ptr<CXTask> getTask();

        //线程池是否退出
        bool is_exit() { return is_exit_; }

        int task_run_count() { return task_run_count_; }
    private:
        //线程池线程的入口函数
        void run();
        int thread_num_ = 0;//线程数量
        std::mutex mux_;
        //线程列表 指针指针版本
        std::vector< std::shared_ptr<std::thread> > threads_;

        //std::list<XTask*> tasks_;
        std::list<std::shared_ptr<CXTask> > tasks_;

        std::condition_variable cv_;
        bool is_exit_ = false; //线程池退出

        //正在运行的任务数量,线程安全
        std::atomic<int> task_run_count_ = { 0 };
    };
}


#endif