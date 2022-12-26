#ifndef SINGLETON_H
#define SINGLETON_H

#include <mutex>

template <typename T>
class CSingleton {
public:
    using pointer = T*;
    CSingleton() = delete;
    CSingleton(const CSingleton&) = delete;
    CSingleton& operator=(const CSingleton&) = delete;

    static T* get_instance() {
        std::call_once(m_instantiated, []() {
            m_instance = new T;
        });
        return m_instance;
    }

    static void destroy() {
        if (m_instance) {
            delete m_instance;
            m_instance = nullptr;
        }
    }

private:
    static pointer m_instance;
    static std::once_flag m_instantiated;
};

template<typename T>
T* CSingleton<T>::m_instance = nullptr;

template<typename T>
std::once_flag CSingleton<T>::m_instantiated;

#endif