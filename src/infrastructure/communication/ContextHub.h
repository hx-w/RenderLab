#ifndef CONTEXTHUB_HPP
#define CONTEXTHUB_HPP

#include <unordered_map>
#include <mutex>
#include <memory>
#include <any>
#include "Service.hpp"
#include "Event.hpp"

namespace fundamental {
    template <typename T>
    using CServiceTable = std::shared_ptr<ServiceTable<std::function<T>>>;

    template <typename T>
    using CService = Service<std::function<T>>;

    template <typename T>
    using CEventTable = std::shared_ptr<EventTable<std::function<T>>>;

    class ContextHub {
    public:
        static ContextHub* getInstance();
        static void destroy();
        ContextHub(const ContextHub&) = delete;
        ContextHub& operator=(const ContextHub&) = delete;
        ~ContextHub() = default;
    
    public:
    // @brief get set of service
    template <typename T>
    typename std::enable_if<std::is_function<T>::value, CServiceTable<T>>::type
    getServiceTable() {
        std::string typeName = typeid(T).name();
        if (m_serviceSet.emplace(typeName, CServiceTable<T>()).second) {
            CServiceTable<T> serviceTable{ new ServiceTable<std::function<T>>() };
            m_serviceSet[typeName] = serviceTable;
        }
        return std::any_cast<CServiceTable<T>>(m_serviceSet[typeName]);
    }

    // @brief get instance of service
    template <typename T>
    typename std::enable_if<std::is_function<T>::value, CService<T>>::type
    getService(const std::string& interfaceName) {
        return getServiceTable<T>()->getFunctor(interfaceName);
    }

    template <typename T>
    typename std::enable_if<std::is_function<T>::value, CEventTable<T>>::type
    getEventTable() {
        std::string typeName = typeid(T).name();
        if (m_eventSet.emplace(typeName, CEventTable<T>()).second) {
            CEventTable<T> eventTable{ new EventTable<std::function<T>>() };
            m_eventSet[typeName] = eventTable;
        }
        return std::any_cast<CEventTable<T>>(m_eventSet[typeName]);
    }

    private:
        ContextHub() = default;
    
    private:
        static ContextHub* m_pInstance;
        static std::once_flag m_inited;
        std::unordered_map<std::string, std::any> m_serviceSet;
        std::unordered_map<std::string, std::any> m_eventSet;
    };
}

#endif