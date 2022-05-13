// 线程安全队列
#ifndef QUEUE_HPP
#define QUEUE_HPP
#include <memory>
#ifndef LOCK_FREE
#include <mutex>
#else
#include <atomic>
#endif

namespace fundamental {
    template <typename T>
    class queue {
#ifndef LOCK_FREE

    public:
        constexpr static bool is_lock_free() {
            return false;
        }
        queue() : m_head(std::make_unique<Node>()),
            m_tail(m_head.get()) {}
        queue(const queue&) = delete;
        queue& operator=(const queue&) = delete;
        ~queue() = default;

        void push(T newValue) {
            auto new_data = std::make_shared<T>(std::move(newValue));
            std::unique_ptr<Node> newNode = std::make_unique<Node>();
            {
                // Resource Acquisition Is Initialization (RAII)
                // 避免手动lock()与unlock()引发的死锁
                std::lock_guard<std::mutex> lk(tail_mutex);
                m_tail->data.swap(new_data);
                auto newTail = newNode.get();
                m_tail->next.swap(newNode);
                m_tail = newTail;
            }
        }

        std::shared_ptr<T> pop() {
            auto oldHead = popHead();
            return oldHead ? oldHead->data : nullptr;
        }

        bool empty() const {
            std::lock(head_mutex, tail_mutex);
            std::lock_guard<std::mutex> headLk(head_mutex, std::adopt_lock);
            std::lock_guard<std::mutex> tailLk(tail_mutex, std::adopt_lock);
            return m_head.get() == m_tail;
        }

    private:
        struct Node {
            std::shared_ptr<T> data;    // 可复制 计数为0时释放内存
            std::unique_ptr<Node> next; // 可移动 不可复制
        };

        std::unique_ptr<Node> popHead() {
            std::lock(head_mutex, tail_mutex); // 死锁避免
            std::lock_guard<std::mutex> headLk(head_mutex, std::adopt_lock);
            std::lock_guard<std::mutex> tailLk(tail_mutex, std::adopt_lock);

            if (m_head.get() == m_tail) {
                return nullptr;
            }

            auto oldHead = std::move(m_head);  // 转换为右值引用，m_head释放
            m_head = std::move(oldHead->next);
            return oldHead;
        }

        mutable std::mutex head_mutex; // 队头锁
        mutable std::mutex tail_mutex; // 队尾锁
        std::unique_ptr<Node> m_head;
        Node* m_tail;
#else

    public:
        constexpr static bool is_lock_free() {
            return true;
        }
#endif
    };
}

#endif
