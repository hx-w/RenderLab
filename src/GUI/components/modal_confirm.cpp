#include <iostream>
#include <imgui.h>
#include "modal_confirm.h"

#include "../engine.h"
#include "../service.h"


using namespace GUISpace;
using namespace RenderSpace;
using namespace std;

#define SERVICE_INST GUIEngine::get_instance()->get_service()


void ModalConfirm::add_notice(const string& name, const string& msg) {
    lock_guard<mutex> lk(st_notice_mutex);

    st_notices.emplace_back(ModalPair(name, msg));
    ImGui::OpenPopup(name.c_str()); /// !! important
}

void ModalConfirm::render(shared_ptr<RenderWindowWidget> win) {
    lock_guard<mutex> lk(st_notice_mutex);

    for (auto iter = st_notices.begin(); iter != st_notices.end();) {
        auto _name = (*iter).first;
        auto _msg = (*iter).second;
        int res = -1;  // -1, no choice; 0, cancel; 1, confirm
        if (ImGui::BeginPopupModal(_name.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text(_msg.c_str());
            //ImGui::Text("All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n");
            ImGui::Separator();

            static bool dont_ask_me_next_time = false;
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
            ImGui::PopStyleVar();

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
