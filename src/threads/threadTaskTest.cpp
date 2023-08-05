#include "threadTask.h"

#include <iostream>
#include "../test_utils.h"

using namespace huffman::parallel::native;

void testSpawnAndTerminateThread() {
    auto thread_initialization = spawnThread();
}

void testOneTask() {
    auto thread_initialization = spawnThread();

    std::function<int()> task = []() { return 5; };
    auto first_task_submit = submitTask(std::move(thread_initialization), task);

    int result = 0;
    auto first_task_result = getResult(std::move(first_task_submit), result);

    assert(result == 5, "Expected result to be 5, but found: ", result);
}

void testTwoTasks() {
    auto thread_initialization = spawnThread();

    std::function<int()> task0 = []() { return 5; };
    auto first_task_submit = submitTask(std::move(thread_initialization), task0);

    int result0 = 0;
    auto first_task_result = getResult(std::move(first_task_submit), result0);

    assert(result0 == 5, "Expected result to be 5, but found: ", result0);

    std::function<float()>  task1 = []() { return 10.5f; };
    auto second_task_submit = submitTask(std::move(first_task_result), task1);

    float result1 = 0.0f;
    auto second_task_result = getResult(std::move(second_task_submit), result1);

    assert(result1 == 10.5f, "Expected result to be 10.5f, but found: ", result1);
}

void testTaskNotRetrieved() {
    auto thread_initialization = spawnThread();

    std::function<int()> task0 = []() { return 5; };
    auto first_task_submit = submitTask(std::move(thread_initialization), task0);
}

void testManualTermination() {
    auto thread_initialization = spawnThread();

    std::function<int()> task = []() { return 5; };
    auto first_task_submit = submitTask(std::move(thread_initialization), task);

    int result = 0;
    auto first_task_result = getResult(std::move(first_task_submit), result);

    assert(result == 5, "Expected result to be 5, but found: ", result);

    closeThread(std::move(first_task_result));
}

void testManualTermination2() {
    auto thread_initialization = spawnThread();

    std::function<int()> task0 = []() { return 5; };
    auto first_task_submit = submitTask(std::move(thread_initialization), task0);

    closeThread(std::move(first_task_submit));
}

void testExecutedNotInMainThread() {
    auto thread_initialization = spawnThread();

    std::function<std::thread::id()>  task = []() { return std::this_thread::get_id(); };
    auto first_task_submit = submitTask(std::move(thread_initialization), task);

    auto main_thread = std::this_thread::get_id();
    auto current_thread = std::this_thread::get_id();
    auto first_task_result = getResult(std::move(first_task_submit), current_thread);

    assert(main_thread != current_thread, "Code has not been run in a non-main thread.");
}

void testMain()
{
    testSpawnAndTerminateThread();
    testOneTask();
    testTwoTasks();
    testTaskNotRetrieved();
    testManualTermination();
    testManualTermination2();
    testExecutedNotInMainThread();
}