#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "task_queue.h"
#include "thread_pool.h"
#include <iostream>

class TaskManager {
public:
    explicit TaskManager(ThreadPool &threadPool) : threadPool(threadPool) {
        max_queue_full_duration = INT_MIN;
        min_queue_full_duration = INT_MAX;
        abandoned_tasks = 0.0;
        current_duration = 0.0;
        lastTime = std::chrono::high_resolution_clock::now();
    }

    ~TaskManager() {
        std::ostringstream task_manager_data;
        task_manager_data << "Min queue overflowing time: " << min_queue_full_duration << " seconds" << std::endl
                          << "Max queue overflowing time: " << max_queue_full_duration << " seconds" << std::endl
                          << "Amount of rejected tasks: " << abandoned_tasks << std::endl;
        std::cout << task_manager_data.str();
    }

    void addTask(int taskId) {
        std::lock_guard<std::mutex> lock(mutex);

        auto currentTime = std::chrono::high_resolution_clock::now();

        if (threadPool.getTaskQueue1().size() < max_queue_size || threadPool.getTaskQueue2().size() < max_queue_size) {
            if (current_duration != 0.0) {
                std::ostringstream queue_add;
                queue_add << Timestamp::getCurrentTimeAsString() << "Tasks can be added again after: "
                << current_duration << " seconds" << std::endl;
                std::cout << queue_add.str();
                if (current_duration > max_queue_full_duration) {
                    max_queue_full_duration = current_duration;
                }
                if (current_duration < min_queue_full_duration) {
                    min_queue_full_duration = current_duration;
                }
                current_duration = 0.0;
            }
            lastTime = currentTime;
            if (threadPool.getTaskQueue1().size() <= threadPool.getTaskQueue2().size()) {
                threadPool.getTaskQueue1().emplace(taskId);
                std::ostringstream queue_add1;
                queue_add1 << Timestamp::getCurrentTimeAsString() << "Task " << taskId << " added to queue 1" << std::endl;
                std::cout << queue_add1.str();
            } else {
                threadPool.getTaskQueue2().emplace(taskId);
                std::ostringstream queue_add2;
                queue_add2 << Timestamp::getCurrentTimeAsString() << "Task " << taskId << " added to queue 2" << std::endl;
                std::cout << queue_add2.str();
            }
        } else {
            std::ostringstream queue_full;
            queue_full << Timestamp::getCurrentTimeAsString() << "Task " << taskId << " rejected. Queues are full" << std::endl;
            std::cout << queue_full.str();
            abandoned_tasks++;
            auto elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastTime);
            double timeSeconds = elapsedTime.count();
            current_duration += timeSeconds;
        }
    }

private:
    static constexpr int max_queue_size = 10;
    double current_duration;
    std::chrono::high_resolution_clock::time_point lastTime;
    double max_queue_full_duration;
    double min_queue_full_duration;
    int abandoned_tasks;
    std::mutex mutex;
    ThreadPool &threadPool;
};

#endif
