#include "threadpool.hpp"

ThreadPool::ThreadPool(size_t thread_count) {
    for (size_t i = 0; i < thread_count; ++i) {
        m_workers.emplace_back([this] { worker_loop(); });
    }
}

ThreadPool::~ThreadPool() { 
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_stop = true;
    }

    m_condition.notify_all();

    for (auto& worker : m_workers) {
        if(worker.joinable())
            worker.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_tasks.push(std::move(task));
    }

    m_condition.notify_one();
}

void ThreadPool::worker_loop() {
    while (true) {
        std::function<void()> task;

        // worker takes task off queue to be run
        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);

            m_condition.wait(lock, [this] {
                return m_stop || !m_tasks.empty();
            });

            // finish remaining tasks before stopping
            if (m_stop  && m_tasks.empty())
                return;
            
            task = std::move(m_tasks.front());
            m_tasks.pop();
        }

        task();
    }
}