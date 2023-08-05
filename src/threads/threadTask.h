#ifndef THREAD_TASK
#define THREAD_TASK

#include <thread>
#include <memory>
#include <future>
#include <functional>

namespace huffman::parallel::native
{
    class threadTask;
    template<typename R> class threadResult;

    inline static threadTask spawnThread();
    template<typename R> inline static threadResult<R> submitTask(threadTask&&, std::packaged_task<R()>&&);
    
    class threadTask {
    private:
        std::thread thread;
        std::unique_ptr<std::promise< std::function<void()> >> task_promise;

        template<typename R> friend class threadResult;
        friend threadTask spawnThread();

    public:
        template<class R>
        inline static threadTask getResult(threadResult<R>&& thread, R& output) {
            return threadTask(std::move(thread), output);
        }

        ~threadTask();

    private:
        threadTask();

        template<class R>
        threadTask(threadResult<R>&& thread, R& output)
            : thread(std::move(thread.thread)), task_promise(std::move(thread.task_promise))
        {
            auto task = std::move(thread.task);
            if( task->valid() )
                output = task->get_future().get();
        }
    };

    template<typename R>
    class threadResult {
    private:
        std::thread thread;
        std::unique_ptr<std::packaged_task<R()>> task;
        std::unique_ptr<std::promise< std::function<void()> >> task_promise;

        friend class threadTask;        
        template<typename T> friend threadResult<T> submitTask(threadTask&&, std::packaged_task<T()>&&);

    public:

        ~threadResult() {
            if( task && task->valid() ) 
                task->get_future().wait();
            if (task_promise)
                task_promise->set_value(std::function<void()>());
            if (thread.joinable())
                thread.join();
        }

    private:
        threadResult(threadTask&& thread, std::packaged_task<R()>&& packaged_task)
            : thread(std::move(thread.thread)),
                task_promise(std::move(thread.task_promise))
        {
            task = std::make_unique<std::packaged_task<R()>>( std::move(packaged_task) );
            auto task_ptr = task.get();
            auto f = [task_ptr](){ (*task_ptr)(); };
            task_promise->set_value(f);
        }
    };

    threadTask spawnThread() {
        return threadTask();
    }

    template<typename R>
    threadResult<R> submitTask(threadTask&& thread, std::packaged_task<R()>&& packaged_task) {
        return threadResult(std::move(thread), std::move(packaged_task));
    }

    inline void closeThread(threadTask&& thread) { }

    template<typename R>
    inline void closeThread(threadResult<R>&& thread) { }
}

#endif