#ifndef AUTOBUS_HPP
#define AUTOBUS_HPP

#include <set>
#include <unordered_map>
#include "ContextHub.h"

namespace fundamental {
    class AutoBus final {
    public:
        template <typename Func, typename _Func, typename... Args>
        void subscribe(const std::string& signal, _Func&& _func, Args&&... args) {
            subscribe<Func>(SignalPolicy::Async, signal, std::forward<_Func>(_func), std::forward<Args>(args)...);
        }

        template <typename Func, typename _Func, typename... Args>
        void subscribe(SignalPolicy policy, const std::string& signal, _Func&& _func, Args&&... args) {
            auto eventTable = ContextHub::getInstance()->getEventTable<Func>();
            auto id = eventTable->subscribe(policy, signal, std::forward<_Func>(_func), std::forward<Args>(args)...);
            auto type = typeid(Func).name();
            if (m_records.find(type) == m_records.end()) {
                m_records[type] = new ImpHelper<Func>();
            }
            m_records[type]->pushEvent(signal, id);
        }

        template <typename Func, typename _Func, typename... Args>
        void registerMethod(const std::string& interfaceName, _Func&& _func, Args&&... args) {
            auto serviceTable = ContextHub::getInstance()->getServiceTable<Func>();
            if (serviceTable->registerMethod(interfaceName, std::forward<_Func>(_func), std::forward<Args>(args)...)) {
                auto type = typeid(Func).name();
                if (m_records.find(type) == m_records.end()) {
                    m_records[type] = new ImpHelper<Func>();
                }
                m_records[type]->pushService(interfaceName);
            }
        }

        AutoBus() = default;
        AutoBus(const AutoBus&) = delete;
        AutoBus& operator=(const AutoBus&) = delete;

        ~AutoBus() {
            for (auto [type, imp] : m_records) {
                delete imp;
            }
            m_records.clear();
        }

    private:
    struct ImpBase {
        void pushService(const std::string& interfaceName) {
            m_serviceSet.insert(interfaceName);
        }

        void pushEvent(const std::string& signal, int id) {
            m_eventSet.emplace(signal, id);
        }

        virtual ~ImpBase() {};
    
    protected:
        std::set<std::string> m_serviceSet;
        std::unordered_multimap<std::string, int> m_eventSet;
    };

    template <typename Func>
    struct ImpHelper : public ImpBase {
        using type = Func;

        ImpHelper() {}

        ~ImpHelper() {
            auto serviceTable = ContextHub::getInstance()->getServiceTable<type>();
            for (auto& service : m_serviceSet) {
                serviceTable->unregister(service);
            }

            auto eventTable = ContextHub::getInstance()->getEventTable<type>();
            for (auto& [signal, id] : m_eventSet) {
                eventTable->unsubscribe(signal, id);
            }
        }
    };

    private:
        std::unordered_map<std::string, ImpBase*> m_records;
    };
}

#endif