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

    static T* instance() {
        std::call_once(m_instantiated, []() {
            pInstance = new T;
        });
        return pInstance;
    }

    static void destroy() {
        if (pInstance) {
            delete pInstance;
            pInstance = nullptr;
        }
    }

private:
    static pointer pInstance;
    static std::once_flag m_instantiated;
};

template<typename T>
T* CSingleton<T>::pInstance = nullptr;

template<typename T>
std::once_flag CSingleton<T>::m_instantiated;

#endif