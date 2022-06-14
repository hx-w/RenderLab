#include "logger.h"
#include "../libs/imgui/imgui.h"

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

Logger* Logger::m_instance = nullptr;
std::once_flag Logger::m_inited;

Logger* Logger::get_instance() {
    std::call_once(m_inited, []() {
        m_instance = new Logger();
    });
    return m_instance;
}

void Logger::destroy() {
    if (m_instance) {
        delete m_instance;
        m_instance = nullptr;
    }
}

void Logger::log(const string& raw_msg, const LOG_LEVEL lvl) {
    if (lvl < m_level) return;
    m_messages.emplace_back(LogMessage(raw_msg, lvl));
    if (m_messages.size() > m_maxsize) {
        m_messages.erase(m_messages.begin());
    }
}

void Logger::flush() {
    m_messages.clear();
}

void Logger::set_level(LOG_LEVEL lvl) {
    m_level = lvl;
}

void Logger::render() {
    ImGui::Begin("Logger");
    for (auto& msg : m_messages) {
        ImGui::Text("%s", msg.message.c_str());
    }
    ImGui::End();
}
