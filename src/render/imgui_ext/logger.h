/**
 * @author hx-w
 * @brief implement a logger widget
 *        to show info by ImGui 
 */
#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <mutex>

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
        static Logger* get_instance();
        static void destroy();
        ~Logger() = default;
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        void log(const std::string& raw_msg, const LOG_LEVEL lvl = LOG_INFO);
        void flush();
        void resize(uint32_t maxsize);
        void set_level(LOG_LEVEL lvl);

        void render();

    private:
        Logger() = default;

    private:
        uint32_t m_maxsize = 8;
        LOG_LEVEL m_level = LOG_INFO;
        std::vector<LogMessage> m_messages;
    
    private:
        static Logger* m_instance;
        static std::once_flag m_inited;
    };

}