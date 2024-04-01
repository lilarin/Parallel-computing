#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <queue>
#include <shared_mutex>
#include <mutex>
#include "timestamp.h"

using read_write_lock = std::shared_mutex;
using read_lock = std::shared_lock<read_write_lock>;
using write_lock = std::unique_lock<read_write_lock>;

struct Task {
    int id;
    std::chrono::high_resolution_clock::time_point task_creation_time;
};

class TaskQueue {
    using TaskQueueImplementation = std::queue<Task>;
public:
    ~TaskQueue() { clear(); }
    bool empty() const;
    size_t size() const;
    void clear();
    bool pop(Task& task);
    void emplace(int id);

private:
    mutable read_write_lock m_rw_lock;
    TaskQueueImplementation m_tasks;
};

bool TaskQueue::empty() const {
    read_lock _(m_rw_lock);
    return m_tasks.empty();
}

size_t TaskQueue::size() const {
    read_lock _(m_rw_lock);
    return m_tasks.size();
}

void TaskQueue::clear() {
    write_lock _(m_rw_lock);
    while (!m_tasks.empty()) {
        m_tasks.pop();
    }
}

bool TaskQueue::pop(Task& task) {
    write_lock _(m_rw_lock);
    if (m_tasks.empty()) {
        return false;
    } else {
        task = m_tasks.front();
        m_tasks.pop();
        return true;
    }
}

void TaskQueue::emplace(int id) {
    write_lock _(m_rw_lock);
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    m_tasks.emplace(id, now);
}

#endif