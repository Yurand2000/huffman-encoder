#include "threadTask.h"

#include <iostream>
#include "../test_utils.h"

using namespace huffman::parallel::native;

void testSpawnAndTerminateThread() {
    auto thread_initialization = spawnThread();
}

void testOneTask() {
    auto thread_initialization = spawnThread();

    auto task = std::packaged_task<int()>([]() { return 5; });
    auto first_task_submit = submitTask(std::move(thread_initialization), std::move(task));

    int result = 0;
    auto first_task_result = threadTask::getResult(std::move(first_task_submit), result);

    assert(result == 5, "Expected result to be 5, but found: ", result);
}

void testTwoTasks() {
    auto thread_initialization = spawnThread();

    auto task0 = std::packaged_task<int()>([]() { return 5; });
    auto first_task_submit = submitTask(std::move(thread_initialization), std::move(task0));

    int result0 = 0;
    auto first_task_result = threadTask::getResult(std::move(first_task_submit), result0);

    assert(result0 == 5, "Expected result to be 5, but found: ", result0);

    auto task1 = std::packaged_task<float()>([]() { return 10.5f; });
    auto second_task_submit = submitTask(std::move(first_task_result), std::move(task1));

    float result1 = 0.0f;
    auto second_task_result = threadTask::getResult(std::move(second_task_submit), result1);

    assert(result1 == 10.5f, "Expected result to be 10.5f, but found: ", result1);
}

void testTaskNotRetrieved() {
    auto thread_initialization = spawnThread();

    auto task0 = std::packaged_task<int()>([]() { return 5; });
    auto first_task_submit = submitTask(std::move(thread_initialization), std::move(task0));
}

void testManualTermination() {
    auto thread_initialization = spawnThread();

    auto task = std::packaged_task<int()>([]() { return 5; });
    auto first_task_submit = submitTask(std::move(thread_initialization), std::move(task));

    int result = 0;
    auto first_task_result = threadTask::getResult(std::move(first_task_submit), result);

    assert(result == 5, "Expected result to be 5, but found: ", result);

    closeThread(std::move(first_task_result));
}

void testManualTermination2() {
    auto thread_initialization = spawnThread();

    auto task0 = std::packaged_task<int()>([]() { return 5; });
    auto first_task_submit = submitTask(std::move(thread_initialization), std::move(task0));

    closeThread(std::move(first_task_submit));
}

void testMain()
{
    testSpawnAndTerminateThread();
    testOneTask();
    testTwoTasks();
    testTaskNotRetrieved();
    testManualTermination();
    testManualTermination2();
}