#ifndef SERVICE_HPP
#define SERVICE_HPP

#include <unordered_map>
#include <string>
#include <functional>
#include <type_traits>
#include <mutex>
#include <future>
#include <thread>
#include <exception>
#include <iostream>
#include "../utilities/ThreadPool.h"

namespace fundamental {
    class ContextHub;
    template <typename Func>
    class ServiceTable;

    template <typename Func>
    class Service {
        friend class ContextHub;

        friend class ServiceTable<Func>;

    public:
        template<typename... Args>
        typename std::invoke_result_t<Func, Args...>
        sync_invoke(Args &&... args) {
            return m_func(std::forward<Args>(args)...);
        }

        template<typename... Args>
        std::future<typename std::invoke_result_t<Func, Args...>>
        async_invoke(Args &&... args) {
            auto fn = std::bind(m_func, std::forward<Args>(args)...);
            return fundamental::ThreadPool::getInstance()->submit(fn);
        }

        operator bool() const {
            return static_cast<bool>(m_func);
        }

    private:
        Service(Func &&func) : m_func(std::move(func)) {}

    private:
        Func m_func;
    };

    template <typename Func>
    class ServiceTable {
        friend class ContextHub;
    public:
        template<typename _Func, typename... Args>
        bool registerMethod(const std::string &interface, _Func &&_func, Args &&... args) {
            return _registerMethod(
                    std::bool_constant<std::is_convertible<_Func, Func>::value>(),
                    interface,
                    std::forward<_Func>(_func),
                    std::forward<Args>(args)...
                    );
        }

        bool unregister(const std::string& interface) {
            std::lock_guard<std::mutex> lk(m_mutex);
            auto iter = m_funcTable.find(interface);
            if (iter != m_funcTable.end()) {
                m_funcTable.erase(iter);
                std::clog << interface << " unregistered\n";
                return true;
            }
            return false;
        }

        template<typename... Args>
        typename std::invoke_result_t<Func, Args...>
        sync_invoke(const std::string& interface, Args&&... args) {
            Service<Func> func = getFunctor(interface);
            if (func) {
                std::clog << interface << " invoked synchronously\n";
                return func.sync_invoke(std::forward<Args>(args)...);
            }
            throw std::runtime_error(interface + " not found");
        }

        template<typename... Args>
        std::future<typename std::invoke_result_t<Func, Args...>>
        async_invoke(const std::string& interface, Args&&... args) {
            Service<Func> func = getFunctor(interface);
            if (func) {
                std::clog << interface << " invoked asynchronously\n";
                return func.async_invoke(std::forward<Args>(args)...);
            }
            throw std::runtime_error(interface + " not found");
        }

    private:
        template<typename _Func, typename... Args>
        bool _registerMethod(std::true_type, const std::string &interface, _Func &&_func, Args &&... args) {
            return impRegisterMethod(interface, std::forward<_Func>(_func));
        }

        template<typename _Func, typename... Args>
        bool _registerMethod(std::false_type, const std::string &interface, _Func &&_func, Args &&... args) {
            Func func = std::bind(std::forward<_Func>(_func), std::forward<Args>(args)...);
            return impRegisterMethod(interface, std::move(func));
        }

        bool impRegisterMethod(const std::string &interface, Func func) {
            std::lock_guard<std::mutex> lk(m_mutex);
            auto ret = m_funcTable.emplace(interface, std::move(func));
            if (ret.second) {
                std::clog << interface << " registered!\n";
            }
            return ret.second;
        }

        ServiceTable() = default;
        ServiceTable(const ServiceTable&) = delete;
        ServiceTable& operator=(const ServiceTable&) = delete;

        Func getFunctor(const std::string& interface) {
            Func func;
            {
                std::lock_guard<std::mutex> lk(m_mutex);
                if (m_funcTable.find(interface) != m_funcTable.end()) {
                    func = m_funcTable.at(interface);
                }
            }
            return func;
        }

    private:
        std::unordered_map<std::string, Func> m_funcTable;
        std::mutex m_mutex;
    };
}

#endif // SERVICE_HPP