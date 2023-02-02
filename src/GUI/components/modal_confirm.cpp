#include <iostream>
#include <imgui.h>
#include "modal_confirm.h"

#include "../engine.h"
#include "../service.h"

using namespace GUISpace;
using namespace std;

#define SERVICE_INST GUIEngine::get_instance()->get_service()


void ModalConfirm::add_notice(const string& title, const string& detail) {
    lock_guard<mutex> lk(st_notice_mutex);

    st_notices.emplace_back(make_unique<NoticeMsg>(title, detail));
}

void ModalConfirm::add_notice(const string& title, function<void(any&&)>&& custom, any&& param) {
    lock_guard<mutex> lk(st_notice_mutex);

    st_notices.emplace_back(make_unique<NoticeMsg>(
        title, forward<decltype(custom)>(custom), forward<decltype(param)>(param)
    ));
}

void ModalConfirm::render() {
    lock_guard<mutex> lk(st_notice_mutex);

    for (auto iter = st_notices.begin(); iter != st_notices.end();) {
        auto _name = (*iter)->title;
        // call in current imgui context
        call_once((*iter)->init, [&]() { ImGui::OpenPopup(_name.c_str()); });

        int res = -1;  // -1, no choice; 0, cancel; 1, confirm
        if (ImGui::BeginPopupModal(_name.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            if ((*iter)->detail.empty()) {
                (*iter)->custom_render(move((*iter)->custom_params));
            }
            else {
				ImGui::Text((*iter)->detail.c_str());
            }
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0))) { res = 1; }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) { res = 0; }
            
            ImGui::EndPopup();
        }
        if (res >= 0) {  // click confirm or cancel
            iter = st_notices.erase(iter);
            ImGui::CloseCurrentPopup();
            /// [Notify] GUI/modal_confirm_feedback
            SERVICE_INST->notify<void(const string&, int)>("/modal_confirm_feedback", _name, res);
        }
        else {
            ++iter;
        }
    }
}
