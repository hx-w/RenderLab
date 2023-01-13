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

#include "base.h"

namespace GUISpace {
    enum LOG_LEVEL {
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

    class Logger: public GUIComponentBase {
    public:
        ~Logger() = default;
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        static void log(const std::string& raw_msg, const LOG_LEVEL lvl = LOG_INFO);
        static void flush();
        static void resize(uint32_t maxsize);
        static void set_level(LOG_LEVEL lvl);

        static void render();

    private:
        Logger() = default;

    };

    static uint32_t st_logger_maxsize = 8;
    static LOG_LEVEL st_logger_level = LOG_INFO;
    static std::vector<LogMessage> st_logger_messages;
    static std::mutex st_logger_mutex;
}