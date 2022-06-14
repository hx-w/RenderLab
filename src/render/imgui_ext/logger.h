/**
 * @author hx-w
 * @brief implement a logger widget
 *        to show info by ImGui 
 */
#pragma once

#include <string>
#include <vector>
#include <ctime>

namespace imgui_ext {
    enum LOG_LEVEL {
        LOG_DEBUG,
        LOG_INFO,
        LOG_WARN,
        LOG_ERROR,
        LOG_FATAL
    };

    struct LogMessage {
        LOG_LEVEL level;
        std::string message;
        std::time_t timestamp;

        LogMessage(const std::string& raw_msg, const LOG_LEVEL lvl);
        ~LogMessage() = default;

        bool operator < (const LogMessage& other) const;
    };

    class Logger {
    public:
        Logger(uint32_t maxsize = 20, LOG_LEVEL lvl = LOG_INFO);
        ~Logger() = default;

        void log(const std::string& raw_msg, const LOG_LEVEL lvl = LOG_INFO);
        void flush();
        void resize(uint32_t maxsize);
        void set_level(LOG_LEVEL lvl);

        void render();

    private:
        uint32_t m_maxsize;
        LOG_LEVEL m_level;
        std::vector<LogMessage> m_messages;
    };

}