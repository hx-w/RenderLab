#pragma once

#include "base.h"

#include <mutex>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <any>

namespace GUISpace {
    class ModalConfirm : public GUIComponentBase {
    public:
        static void render();
    
        static void add_notice(const std::string&, const std::string&);
        /// add custom function and params when display notice
        static void add_notice(
            const std::string&,
            std::function<void(std::any&&)>&&,
            std::any&&
        );
    };


    struct NoticeMsg {
        NoticeMsg(const std::string& t, const std::string& d):
            title(t), detail(d) {}

        NoticeMsg(
            const std::string& t,
            std::function<void(std::any&&)>&& func,
            std::any&& param
        ): title(t), custom_render(func), custom_params(param) {}

        std::string title = "";

        std::string detail = "";

        std::function<void(std::any&&)> custom_render;
        std::any custom_params;

        std::once_flag init;
    };

    static std::vector<std::unique_ptr<NoticeMsg>> st_notices;

    static std::mutex st_notice_mutex;
}
