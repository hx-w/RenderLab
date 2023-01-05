
#ifndef GUI_ENGINE_H
#define GUI_ENGINE_H

#include <mutex>
#include <set>

namespace GUISpace {
    // singleton
    // 包括所有service的资源管理
    class GUIService;
    class GUIEngine {
    public:
        static GUIEngine* get_instance();
        static void destroy();
        GUIEngine(const GUIEngine&) = delete;
        GUIEngine& operator=(const GUIEngine&) = delete;
        ~GUIEngine() { terminate(); };

        GUIService* create_service();
        void destroy_service(GUIService*);

    private:
        GUIEngine() = default;
        void terminate();

    private:
        std::set<GUIService*> m_services;

    private:
        static GUIEngine* m_instance;
        static std::once_flag m_inited;
    };

    GUIService* make_service();
}

#endif
