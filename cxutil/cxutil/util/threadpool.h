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
        /// ��ʼ���̳߳�
        /// @para num �߳�����
        void init(int num);

        //////////////////////////////////////////////
        /// ���������̣߳������ȵ���Init
        void start();

        //////////////////////////////////////////////
        /// �̳߳��˳�
        void stop();

        //void AddTask(XTask* task);
        void addTask(std::shared_ptr<CXTask> task);

        std::shared_ptr<CXTask> getTask();

        //�̳߳��Ƿ��˳�
        bool is_exit() { return is_exit_; }

        int task_run_count() { return task_run_count_; }
    private:
        //�̳߳��̵߳���ں���
        void run();
        int thread_num_ = 0;//�߳�����
        std::mutex mux_;
        //�߳��б� ָ��ָ��汾
        std::vector< std::shared_ptr<std::thread> > threads_;

        //std::list<XTask*> tasks_;
        std::list<std::shared_ptr<CXTask> > tasks_;

        std::condition_variable cv_;
        bool is_exit_ = false; //�̳߳��˳�

        //�������е���������,�̰߳�ȫ
        std::atomic<int> task_run_count_ = { 0 };
    };
}


#endif