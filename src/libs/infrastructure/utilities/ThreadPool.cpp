#include "ThreadPool.h"

namespace fundamental {
    using namespace std;

    constexpr int CORE_NUM = 20;

    once_flag ThreadPool::m_inited;
    ThreadPool* ThreadPool::m_pInstance = nullptr;

    ThreadPool* ThreadPool::getInstance() {
        call_once(m_inited, []() {
            m_pInstance = new ThreadPool;
            });
        return m_pInstance;
    }

    void ThreadPool::destroy() {
        if (m_pInstance) {
            delete m_pInstance;
            m_pInstance = nullptr;
        }
    }

    void ThreadPool::execute() {
        // 从队列中获取任务执行
        shared_ptr<TaskWrapper> task;
        while (!m_done) {
            unique_lock<mutex> lk(m_mutex);
            m_conVar.wait(lk, [this]() { return m_done || !m_taskQueue.empty(); });
            task = m_taskQueue.pop();
			if (task) {
				(*task)();
			}
        }
    }

    ThreadPool::ThreadPool() : m_done(false) {
        // 根据硬件资源启动一定数量线程放入集合
        const auto core = thread::hardware_concurrency() == 0 ?
            CORE_NUM : thread::hardware_concurrency();
        try {
            for (auto idx = 0U; idx < core; ++idx)
                m_threads.emplace_back(&ThreadPool::execute, this);
        }
        catch (...) {
            m_done = true;
            m_conVar.notify_all();
            throw;
        }
    }

    ThreadPool::~ThreadPool() {
        m_done = true;
        m_conVar.notify_all();
        for (auto& th : m_threads) {
            if (th.joinable()) {
                th.join();
            }
        }
    }
}