#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <mutex>
#include <memory>
#include <thread>
#include <vector>
#include <type_traits>
#include <condition_variable>
#include <future>
#include <atomic>
#include "../container/queue.hpp"

namespace fundamental {
    class ThreadPool {
    public:
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ~ThreadPool();

        static ThreadPool* getInstance();
        static void destroy();

        // 提交任务
        template<typename Func>
        typename std::enable_if_t<std::is_invocable<Func>::value,
            std::future<typename std::invoke_result_t<Func>>>
        submit(Func func) {
            using result_type = typename std::invoke_result_t<Func>;
            std::packaged_task<result_type()> task(std::move(func));
            std::future<result_type> res = task.get_future();
            m_taskQueue.push(std::move(task));
            // m_taskQueue.push(std::move(TaskWrapper(std::move(func))));
            m_conVar.notify_one();

            return std::move(res);
        }

    private:
        class TaskWrapper {
        public:
            template<typename Func>
            TaskWrapper(Func&& func) :
                m_imp(new impl_type<Func>(std::move(func))) {}
            TaskWrapper() = default;
            TaskWrapper(const TaskWrapper&) = delete;
            TaskWrapper& operator=(const TaskWrapper&) = delete;
            TaskWrapper(TaskWrapper&& other) :
                m_imp(std::move(other.m_imp)) {}
            TaskWrapper& operator=(TaskWrapper&& other) {
                m_imp.swap(other.m_imp);
                return *this;
            }
            void operator()() {
                m_imp->call();
            }

        private:
            struct impl_base {
                virtual void call() = 0;
                virtual ~impl_base() = default;
            };
            template<typename Func>
            struct impl_type: public impl_base {
                impl_type(Func&& func):
                    m_functor(std::move(func)) {}
                void call() override {
                    m_functor();
                }
                Func m_functor;
            };

        private:
            std::unique_ptr<impl_base> m_imp;
        };
        void execute();
        ThreadPool();
    
    private:
        static std::once_flag m_inited;
        static ThreadPool* m_pInstance;

        std::atomic<bool> m_done;
        std::vector<std::thread> m_threads;
        std::mutex m_mutex;
        std::condition_variable m_conVar;
        fundamental::queue<TaskWrapper> m_taskQueue;
    };
}

#endif