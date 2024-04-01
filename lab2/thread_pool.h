#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "task_queue.h"
#include <iostream>
#include <condition_variable>
#include <random>

class ThreadPool {
public:
    ~ThreadPool() {
        stop();
        taskQueue1.clear();
        taskQueue2.clear();
        std::ostringstream stop;
        stop << "Thread pool stopped" << std::endl
             << "Total idle time: " << totalThreadsIdleTime << " seconds" << std::endl
             << "Total thread amount: " << poolThreadAmount << std::endl
             << "Average idle time for thread: " << totalThreadsIdleTime / poolThreadAmount << " seconds" << std::endl
             << "Total done task amount: " << tasksAmount << std::endl
             << "Average idle time for task: " << totalTasksIdleTime / tasksAmount << " seconds" << std::endl;
        std::cout << stop.str();
    }

    void start() {
        for (int i = 0; i < 2; ++i) {
            workers.emplace_back(std::thread(&ThreadPool::workerFunc, this, std::ref(taskQueue1), std::ref(mutex1), std::ref(cv1)));
            workers.emplace_back(std::thread(&ThreadPool::workerFunc, this, std::ref(taskQueue2), std::ref(mutex2), std::ref(cv2)));
            poolThreadAmount += 2;
        }
    }

    void pause() {
        std::unique_lock<std::mutex> lock(mutex_pause);
        paused = true;
        std::ostringstream pause;
        pause << Timestamp::getCurrentTimeAsString() << "Thread pool paused. New tasks will not be taken" << std::endl;
        std::cout << pause.str();
        cv1.notify_all();
        cv2.notify_all();
    }

    void resume() {
        std::unique_lock<std::mutex> lock(mutex_pause);
        paused = false;
        std::ostringstream resume;
        resume << Timestamp::getCurrentTimeAsString() << "Thread pool resumed" << std::endl;
        std::cout << resume.str();
        cv1.notify_all();
        cv2.notify_all();
    }

    void stop() {
        running = false;
        cv1.notify_all();
        cv2.notify_all();

        for (auto& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    TaskQueue& getTaskQueue1() {
        return taskQueue1;
    }

    TaskQueue& getTaskQueue2() {
        return taskQueue2;
    }

private:
    void workerFunc(TaskQueue& queue, std::mutex& mutex, std::condition_variable& cv) {
        while (running) {
            std::unique_lock<std::mutex> thread_lock(mutex);
            auto startWaitTime = std::chrono::high_resolution_clock::now();

            cv.wait(thread_lock, [&]() {
                return !running || !paused || !queue.empty();
            });

            auto endWaitTime = std::chrono::high_resolution_clock::now();
            auto elapsedWaitTime = std::chrono::duration_cast<std::chrono::duration<double>>(endWaitTime - startWaitTime);

            if (!paused && !queue.empty()) {
                Task task;
                queue.pop(task);
                thread_lock.unlock();

                std::chrono::high_resolution_clock::time_point task_pop_time = std::chrono::high_resolution_clock::now();
                auto elapsed_creation = duration_cast<std::chrono::duration<double>>(task_pop_time - task.task_creation_time);
                double task_queue_time = elapsed_creation.count();
                totalTasksIdleTime += task_queue_time;
                tasksAmount ++;

                std::ostringstream claim;
                claim << Timestamp::getCurrentTimeAsString() << "Task " << task.id << " claimed. Wait time is: " << task_queue_time << " seconds" << std::endl;
                std::cout << claim.str();

                std::this_thread::sleep_for(std::chrono::seconds(getRandomValue(6, 10)));

                std::ostringstream complete;
                complete << Timestamp::getCurrentTimeAsString() << "Task " << task.id << " completed" << std::endl;
                std::cout << complete.str();
            } else {
                std::lock_guard<std::mutex> wait_lock(mutex_totalIdleTime);
                totalThreadsIdleTime += elapsedWaitTime.count();
            }
        }
    }

    static int getRandomValue(int lower_bound, int upper_bound) {
        std::random_device rd;
        std::uniform_int_distribution<int> dist(lower_bound, upper_bound);
        return dist(rd);
    }

private:
    std::vector<std::thread> workers;
    TaskQueue taskQueue1;
    TaskQueue taskQueue2;
    std::mutex mutex1;
    std::mutex mutex2;
    std::condition_variable cv1;
    std::condition_variable cv2;
    std::mutex mutex_pause;
    double totalThreadsIdleTime = 0.0;
    double totalTasksIdleTime = 0.0;
    int poolThreadAmount = 0;
    int tasksAmount = 0;
    std::mutex mutex_totalIdleTime;
    bool running = true;
    bool paused = false;
};

#endif
