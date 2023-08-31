#include "threadTask.h"

namespace huffman::parallel::native::detail
{
    void workerThreadFunction(std::promise< std::function<void()> >* promise) {
        auto future = promise->get_future();
        while (true) {
            auto function = future.get();

            if (!function) return;

            //promises must be used only once, so a new object is created for the next iteration
            *promise = std::promise<std::function<void()>>();
            future = promise->get_future();

            function();            
        };
    }
}

namespace huffman::parallel::native
{
    threadTask::~threadTask() {
        if (task_promise)
            task_promise->set_value(std::function<void()>());
        if (thread.joinable())
            thread.join();
    }

    threadTask spawnThread() {
        auto thread = threadTask();
        thread.task_promise = std::make_unique< std::promise<std::function<void()>> >();
        thread.thread = std::thread(detail::workerThreadFunction, thread.task_promise.get());
        return thread;
    }
}