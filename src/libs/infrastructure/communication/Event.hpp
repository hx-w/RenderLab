#ifndef EVENT_HPP
#define EVENT_HPP

#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <thread>
#include <algorithm>
#include <iostream>
#include <memory>
#include <functional>
#include "../utilities/ThreadPool.h"

namespace fundamental {
    class ContextHub;
    enum class SignalPolicy {
        Sync,
        Async
    };

    template <typename Func>
    class EventTable {
        friend class ContextHub;
    public:
        // listen intrested signal
        template<typename _Func, typename... Args>
        int subscribe(const std::string& signal, _Func&& _func, Args&& ... args) {
            return _subscribe(SignalPolicy::Async, signal, std::forward<_Func>(_func), std::forward<Args>(args)...);
        }

        template<typename _Func, typename... Args>
        int subscribe(SignalPolicy policy, const std::string& signal, _Func&& _func, Args&& ... args) {
            return _subscribe(
                std::bool_constant<std::is_convertible<_Func, Func>::value>(),
                policy,
                signal, std::forward<_Func>(_func), std::forward<Args>(args)...
            );
        }

        template<typename... Args>
        std::vector<std::future<void>> notify(const std::string& signal, Args&&... args) {
#ifdef _DEBUG_
            std::clog << "broadcast signal: " << signal << std::endl;
#endif
            auto vtrFunc = getFuncs(signal);
            std::vector<std::future<void>> rets;

            for (auto& func : vtrFunc) {
                if (func.m_policy == SignalPolicy::Async) {
                    auto fn = std::bind(func, std::forward<Args>(args)...);
                    rets.emplace_back(fundamental::ThreadPool::getInstance()->submit(fn));
                }
                else {
                    try {
                        func(std::forward<Args>(args)...);
                    }
                    catch (const std::exception &ept) {
                        std::clog << "error occurs when broadcast " << signal << ", " << ept.what() << std::endl;
                    }
                    catch (...) {
                        std::clog << "error occurs when broadcast " << signal << std::endl;
                    }
                }
            }
            return std::move(rets);
        }

        void unsubscribe(const std::string& signal, int id) {
            std::lock_guard<std::mutex> lk(m_mutex);
            if (m_funcTable.find(signal) != m_funcTable.end()) {
                auto& vtr = m_funcTable[signal];
                auto iter = std::find_if(vtr.begin(), vtr.end(), [id](const FuncType& func) {
                    return func.m_id == id;
                });
                if (iter != vtr.end()) {
                    vtr.erase(iter);
                    std::clog << "unsubscribe " << signal << std::endl;
                }
            }
        }

    private:
        struct FuncType {
                FuncType(int id, Func func, SignalPolicy policy):
                    m_id(id), m_func(func), m_policy(policy) {}
                template<typename... Args>
                void operator()(Args&&... args) {
                    m_func(std::forward<Args>(args)...);
                }
                int m_id;
                Func m_func;
                SignalPolicy m_policy;
            };

        std::vector<FuncType> getFuncs(const std::string& signal) {
            std::vector<FuncType> ret;
            std::lock_guard<std::mutex> lk(m_mutex);
            if (m_funcTable.find(signal) != m_funcTable.end()) {
                ret = m_funcTable.at(signal);
            }
            return ret;
        }

        template<typename _Func, typename... Args>
        int _subscribe(std::true_type, SignalPolicy policy, const std::string& signal, _Func&& _func, Args&& ... args) {
            FuncType element(m_id, _func, policy);
            return impSubscribe(signal, std::move(element));
        }

        template<typename _Func, typename... Args>
        int _subscribe(std::false_type, SignalPolicy policy, const std::string& signal, _Func&& _func, Args&& ... args) {
            Func func = std::bind(std::forward<_Func>(_func), std::forward<Args>(args)...);
            FuncType element(m_id, func, policy);

            return impSubscribe(signal, std::move(element));
        }

        int impSubscribe(const std::string& signal, FuncType functor) {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_funcTable[signal].emplace_back(std::move(functor));
            std::clog << "subscribe " << signal << std::endl;
            return m_id++;
        }

        EventTable() = default;
        EventTable(const EventTable&) = delete;
        EventTable& operator=(const EventTable&) = delete;

    private:
        int m_id = 0;
        std::mutex m_mutex;
        std::unordered_map<std::string, std::vector<FuncType>> m_funcTable;
    };
}

#endif