#include "task_manager.h"
#include <random>

std::mutex taskNumberMutex;
int nextTaskNumber = 1;

class TaskManagerThread : public std::thread {
public:
    explicit TaskManagerThread(TaskManager& manager) : std::thread(&TaskManagerThread::run, this, std::ref(manager)) {}

    void run(TaskManager& manager) {
        for (int i = 1; i <= getRandomValue(15, 20); ++i) {
            int taskNumber;
            {
                std::lock_guard<std::mutex> lock(taskNumberMutex);
                taskNumber = nextTaskNumber++;
            }
            manager.addTask(taskNumber);
            std::this_thread::sleep_for(std::chrono::seconds(getRandomValue(1, 2)));
        }
    }

private:
    static int getRandomValue(int lower_bound, int upper_bound) {
        std::random_device rd;

        std::uniform_int_distribution<int> dist(lower_bound, upper_bound);

        return dist(rd);
    }
};

class ThreadPoolManager {
public:
    explicit ThreadPoolManager(ThreadPool& pool) : thread_pool(pool) {}

    void run() {
        thread_pool.start();
    }

    void pause() {
        thread_pool.pause();
    }

    void resume() {
        thread_pool.resume();
    }

    void stop() {
        thread_pool.stop();
    }

private:
    ThreadPool& thread_pool;
};

int main() {
    ThreadPool threadPool;
    TaskManager taskManager(threadPool);
    ThreadPoolManager threadPoolManager(threadPool);

    std::vector<TaskManagerThread> taskManagerThreads;

    for (int i = 0; i < 2; ++i) {
        taskManagerThreads.emplace_back(taskManager);
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::thread poolThread(&ThreadPoolManager::run, &threadPoolManager);
    std::this_thread::sleep_for(std::chrono::seconds(5));

    threadPoolManager.pause();
    std::this_thread::sleep_for(std::chrono::seconds(10));

    threadPoolManager.resume();
    std::this_thread::sleep_for(std::chrono::seconds(60));

    threadPoolManager.stop();

    for (auto& thread : taskManagerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    if (poolThread.joinable()) {
        poolThread.join();
    }



    return 0;
}