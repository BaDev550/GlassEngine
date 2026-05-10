#pragma once
#include "Core.h"
#include "Memory/Memory.h"

#include <format>
#include <chrono>
#include <initializer_list>
#include <sstream>
#include <fstream>

namespace ge {
    enum logLevel {
        LL_Info = 0,
        LL_Error,
        LL_Trace,
        LL_Warn,
        LL_Critical
    };

    constexpr std::string_view LogTypeToString(logLevel lvl) {
        switch (lvl) {
        case LL_Error:    return "Error";
        case LL_Info:     return "Info";
        case LL_Trace:    return "Trace";
        case LL_Warn:     return "Warn";
        case LL_Critical: return "Critical";
        default:          return "INVALID_TYPE";
        }
    }

    template<typename... Args>
    std::string FormatMessage(std::format_string<Args...> fmt, Args&&... args) {
        return std::vformat(fmt.get(), std::make_format_args(args...));
    }

#define GE_LOG_MESSAGE_FORMAT(msg, type, name) std::format("[{}][{}]: {}", name, LogTypeToString(type), msg)
#define GE_LOG_TO_CONSOLE(msg, type, name) std::cout << GE_LOG_MESSAGE_FORMAT(msg, type, name) << '\n';
#define GE_LOG_TO_SINK(msg, type, name, sink) if(sink) sink->Log(GE_LOG_MESSAGE_FORMAT(msg, type, name), type);
#define GE_LOG(type, msg, ...) auto Fmsg = FormatMessage(msg, __VA_ARGS__); GE_LOG_TO_CONSOLE(Fmsg, type, "DEFAULT_CONSOLE_LOGGER");

    class Sink : public mem::RefCounted {
    public:
        Sink(const std::string& name, const GEVector<logLevel>& acceptedLevels) : _name(name), _acceptedLevels(acceptedLevels) {}
        void Log(const std::string& message, logLevel level) {
            if (std::find(_acceptedLevels.begin(), _acceptedLevels.end(), level) != _acceptedLevels.end()) {
                _messages.push_back(message);
            }
        }
        void Clear() {
            _messages.clear();
        }
        void Dump(const std::string& filepath) {
            std::ofstream file(filepath); // TODO (badev): create its own class
            for (const auto& msg : _messages) {
                file << msg << '\n';
            }
            file.close();
        }
        GEVector<std::string> GetMessages() const { return _messages; }
    private:
        std::string _name;
        GEVector<logLevel> _acceptedLevels;
        GEVector<std::string> _messages;
    };

    class Logger : public mem::RefCounted {
    public:
        Logger(const char* name) : _name(name) {}
        Logger(const char* name, mem::Ref<Sink> sink) : _name(name), _sink(sink) {}
        ~Logger() = default;

        template<typename... Args>
        void info(std::format_string<Args...> fmt, Args&&... args) {
            auto msg = FormatMessage(fmt, std::forward<Args>(args)...);
            GE_LOG_TO_CONSOLE(msg, LL_Info, _name);
            GE_LOG_TO_SINK(msg, LL_Info, _name, _sink);
        }

        template<typename... Args>
        void trace(std::format_string<Args...> fmt, Args&&... args) {
            auto msg = FormatMessage(fmt, std::forward<Args>(args)...);
            GE_LOG_TO_CONSOLE(msg, LL_Trace, _name);
            GE_LOG_TO_SINK(msg, LL_Trace, _name, _sink);
        }

        template<typename... Args>
        void warn(std::format_string<Args...> fmt, Args&&... args) {
            auto msg = FormatMessage(fmt, std::forward<Args>(args)...);
            GE_LOG_TO_CONSOLE(msg, LL_Warn, _name);
            GE_LOG_TO_SINK(msg, LL_Warn, _name, _sink);
        }

        template<typename... Args>
        void error(std::format_string<Args...> fmt, Args&&... args) {
            auto msg = FormatMessage(fmt, std::forward<Args>(args)...);
            GE_LOG_TO_CONSOLE(msg, LL_Error, _name);
            GE_LOG_TO_SINK(msg, LL_Error, _name, _sink);
        }

        template<typename... Args>
        void critical(std::format_string<Args...> fmt, Args&&... args) {
            auto msg = FormatMessage(fmt, std::forward<Args>(args)...);
            GE_LOG_TO_CONSOLE(msg, LL_Critical, _name);
            GE_LOG_TO_SINK(msg, LL_Critical, _name, _sink);
        }

    private:
        const char* _name;
        mem::Ref<Sink> _sink = nullptr;
    };
}