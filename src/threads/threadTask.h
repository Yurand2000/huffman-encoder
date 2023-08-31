#ifndef THREAD_TASK
#define THREAD_TASK

#include <thread>
#include <memory>
#include <future>
#include <functional>

namespace huffman::parallel::native
{
    class threadTask;
    template<class R, class ...ArgTypes> class threadResult;

    threadTask spawnThread();
    template<class R, class ...ArgTypes> inline threadResult<R, ArgTypes...> submitTask(threadTask&&, std::function<R(ArgTypes...)>, ArgTypes...);
    template<class R, class ...ArgTypes> inline threadTask getResult(threadResult<R, ArgTypes...>&&, R&);
    
    //a threadTask object owns a thread which is waiting for a new task.
    class threadTask {
    private:
        std::thread thread;
        std::unique_ptr<std::promise< std::function<void()> >> task_promise;

        template<class R, class ...ArgTypes> friend class threadResult;
        friend threadTask spawnThread();
        template<class R, class ...ArgTypes> friend threadTask getResult(threadResult<R, ArgTypes...>&&, R&);

    public:
        threadTask() = default;
        threadTask(threadTask&&) = default;
        threadTask& operator=(threadTask&&) = default;

        ~threadTask();

    private:
        template<class R, class ...ArgTypes>
        threadTask(threadResult<R, ArgTypes...>&& thread, R& output)
            : thread(std::move(thread.thread)), task_promise(std::move(thread.task_promise))
        {
            auto task = std::move(thread.task);
            if( task->valid() )
                output = task->get_future().get();
        }
    };

    //a threadResult object owns a thread which is executing a task.
    template<class R, class ...ArgTypes>
    class threadResult {
    private:
        std::thread thread;
        std::unique_ptr<std::packaged_task<R(ArgTypes...)>> task;
        std::unique_ptr<std::promise< std::function<void()> >> task_promise;

        friend class threadTask;        
        template<class R0, class ...ArgTypes0> friend threadResult<R0, ArgTypes0...> submitTask(threadTask&&, std::function<R0(ArgTypes0...)>, ArgTypes0...);

    public:
        threadResult() = default;
        threadResult(threadResult&&) = default;
        threadResult& operator=(threadResult&&) = default;

        ~threadResult() {
            if( task && task->valid() ) 
                task->get_future().wait();
            if (task_promise)
                task_promise->set_value(std::function<void()>());
            if (thread.joinable())
                thread.join();
        }

    private:
        threadResult(threadTask&& thread, std::function<R(ArgTypes...)> function, ArgTypes... args)
            : thread(std::move(thread.thread)),
                task_promise(std::move(thread.task_promise))
        {
            task = std::make_unique<std::packaged_task<R(ArgTypes...)>>( function );
            auto task_ptr = task.get();
            auto f = [task_ptr, args...](){ (*task_ptr)(args...); };
            task_promise->set_value(f);
        }
    };

    template<class R, class ...ArgTypes>
    threadResult<R, ArgTypes...> submitTask(threadTask&& thread, std::function<R(ArgTypes...)> function, ArgTypes... args) {
        return threadResult(std::move(thread), function, args...);
    }

    template<class R, class ...ArgTypes>
    threadTask getResult(threadResult<R, ArgTypes...>&& thread, R& output) {
        return threadTask(std::move(thread), output);
    }

    inline void closeThread(threadTask&& thread) { }

    template<class R, class ...ArgTypes>
    inline void closeThread(threadResult<R, ArgTypes...>&& thread) { }
}

#endif