#include "logger.h"
#include <imgui.h>

using namespace imgui_ext;
using namespace std;

LogMessage::LogMessage(const string& raw_msg, const LOG_LEVEL lvl) :
    level(lvl),
    timestamp(std::time(nullptr)) {
    static const string levels[] = {
        "DEBUG",
        "INFO",
        "WARN",
        "ERROR",
        "FATAL"
    };

    tm *ltm = localtime(&timestamp);
    char buffer[128] = "";
    snprintf(
        buffer, sizeof(buffer), 
        "[%s] %02d:%02d:%02d %s",
        levels[static_cast<int>(lvl)].c_str(), 
        ltm->tm_hour, ltm->tm_min, ltm->tm_sec,
        raw_msg.c_str()
    );
    message = string(buffer);
}

bool LogMessage::operator < (const LogMessage& other) const {
    return timestamp < other.timestamp;
}

void Logger::log(const string& raw_msg, const LOG_LEVEL lvl) {
    if (lvl < st_logger_level) return;
    st_logger_messages.emplace_back(LogMessage(raw_msg, lvl));
    if (st_logger_messages.size() > st_logger_maxsize) {
        st_logger_messages.erase(st_logger_messages.begin());
    }
}

void Logger::flush() {
    st_logger_messages.clear();
}

void Logger::set_level(LOG_LEVEL lvl) {
    st_logger_level = lvl;
}

void Logger::render() {
    ImGui::Begin("Logger");
    for (auto& msg : st_logger_messages) {
        ImVec4 clr = ImVec4(0.7, 0.7, 0.7, 1.);
        switch (msg.level) {
        case LOG_ERROR:
            clr = ImVec4(0.9, .0, .0, 1.);
            break;
        case LOG_FATAL:
            clr = ImVec4(1.0, .0, .0, 1.);
            break;
        case LOG_INFO:
            clr = ImVec4(0.7, 0.8, 0.3, 1.);
            break;
        case LOG_WARN:
            clr = ImVec4(0.8, 1.0, 0.0, 1.);
            break;
        default: break;
        }
        ImGui::TextColored(clr, "%s", msg.message.c_str());
    }
    ImGui::End();
}
