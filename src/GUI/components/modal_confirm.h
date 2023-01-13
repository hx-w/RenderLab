#pragma once

#include "base.h"

#include <mutex>
#include <string>
#include <vector>

namespace GUISpace {
    class ModalConfirm : public GUIComponentBase {
    public:
        static void render();
    
        static void add_notice(const std::string&, const std::string&);
    };

    using ModalPair = std::pair<std::string, std::string>;
    static std::vector<ModalPair> st_notices;

    static std::mutex st_notice_mutex;
}
