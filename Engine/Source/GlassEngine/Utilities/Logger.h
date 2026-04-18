#pragma once

#include "GlassEngine/Core/Memory.h"
#include "GlassEngine/Utilities/Time.h"
#include "GlassEngine/Utilities/Console.h"
#include <format>
#include <chrono>

#ifdef _DEBUG
#define GE_CORE_TRACE(...) ge::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define GE_CORE_INFO(...) ge::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define GE_CORE_WARN(...) ge::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define GE_CORE_ERROR(...) ge::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define GE_CORE_CRITICAL(...) ge::Logger::GetCoreLogger()->critical(__VA_ARGS__)

#define GE_APPLICATION_TRACE(...) ge::Logger::GetApplicationLogger()->trace(__VA_ARGS__)
#define GE_APPLICATION_INFO(...) ge::Logger::GetApplicationLogger()->info(__VA_ARGS__)
#define GE_APPLICATION_WARN(...) ge::Logger::GetApplicationLogger()->warn(__VA_ARGS__)
#define GE_APPLICATION_ERROR(...) ge::Logger::GetApplicationLogger()->error(__VA_ARGS__)

#define GE_GRAPHICS_TRACE(...) ge::Logger::GetGraphicsLogger()->trace(__VA_ARGS__)
#define GE_GRAPHICS_INFO(...) ge::Logger::GetGraphicsLogger()->info(__VA_ARGS__)
#define GE_GRAPHICS_WARN(...) ge::Logger::GetGraphicsLogger()->warn(__VA_ARGS__)
#define GE_GRAPHICS_ERROR(...) ge::Logger::GetGraphicsLogger()->error(__VA_ARGS__)
#define GE_GRAPHICS_CRITICAL(...) ge::Logger::GetGraphicsLogger()->critical(__VA_ARGS__)

#define GE_SCRIPT_TRACE(...) ge::Logger::GetScriptingLogger()->trace(__VA_ARGS__)
#define GE_SCRIPT_INFO(...) ge::Logger::GetScriptingLogger()->info(__VA_ARGS__)
#define GE_SCRIPT_WARN(...) ge::Logger::GetScriptingLogger()->warn(__VA_ARGS__)
#define GE_SCRIPT_ERROR(...) ge::Logger::GetScriptingLogger()->error(__VA_ARGS__)
#else 
#define GE_CORE_TRACE(...)
#define GE_CORE_INFO(...)
#define GE_CORE_WARN(...)
#define GE_CORE_ERROR(...)
#define GE_CORE_CRITICAL(...)
#define GE_APPLICATION_TRACE(...)
#define GE_APPLICATION_INFO(...)
#define GE_APPLICATION_WARN(...)
#define GE_APPLICATION_ERROR(...)
#define GE_GRAPHICS_TRACE(...)
#define GE_GRAPHICS_INFO(...)
#define GE_GRAPHICS_WARN(...)
#define GE_GRAPCHICS_ERROR(...)
#define GE_GRAPHICS_CRITICAL(...)
#define GE_SCRIPT_TRACE(...)
#define GE_SCRIPT_INFO(...)
#define GE_SCRIPT_WARN(...)
#define GE_SCRIPT_ERROR(...)
#endif

namespace ge {
    namespace log {
        enum logLevel {
            LL_Error,
            LL_Info,
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
            auto tup = std::make_tuple(std::forward<Args>(args)...);
            return std::apply([&](auto&... a) { return std::vformat(fmt.get(), std::make_format_args(a...)); }, tup);
        }

#define GE_FORMAT_LOG_MESSAGE(msg, type, name) std::format("[{}][{}][{}]: {}", Time::GetCurrentLocalTime(), name, LogTypeToString(type), msg)
#define GE_LOG(msg, type, name) std::cout << GE_FORMAT_LOG_MESSAGE(msg, type, name) << std::endl; ge::Console::Get().Log(msg);

        class Logger : public mem::RefCounted {
        public:
            Logger(const char* name) : _name(name) {}
            ~Logger() = default;

            template<typename... Args>
            void info(std::format_string<Args...> fmt, Args&&... args) {
                auto msg = FormatMessage(fmt, std::forward<Args>(args)...);
                GE_LOG(msg, LL_Info, _name);
            }

            template<typename... Args>
            void trace(std::format_string<Args...> fmt, Args&&... args) {
                auto msg = FormatMessage(fmt, std::forward<Args>(args)...);
                GE_LOG(msg, LL_Trace, _name);
            }

            template<typename... Args>
            void warn(std::format_string<Args...> fmt, Args&&... args) {
                auto msg = FormatMessage(fmt, std::forward<Args>(args)...);
                GE_LOG(msg, LL_Warn, _name);
            }

            template<typename... Args>
            void error(std::format_string<Args...> fmt, Args&&... args) {
                auto msg = FormatMessage(fmt, std::forward<Args>(args)...);
                GE_LOG(msg, LL_Error, _name);
            }

            template<typename... Args>
            void critical(std::format_string<Args...> fmt, Args&&... args) {
                auto msg = FormatMessage(fmt, std::forward<Args>(args)...);
                GE_LOG(msg, LL_Critical, _name);
            }

        private:
            const char* _name;
        };
    }

    class Logger {
    public:
        static void Init() {
            s_coreLogger = mem::Ref<log::Logger>::Create("CORE");
            s_graphicsLogger = mem::Ref<log::Logger>::Create("GRAPHICS");
            s_applicationLogger = mem::Ref<log::Logger>::Create("APPLICATION");
            s_scriptingLogger = mem::Ref<log::Logger>::Create("SCRIPT");
        }
        static void Destroy() {
            s_coreLogger = nullptr;
            s_graphicsLogger = nullptr;
            s_applicationLogger = nullptr;
            s_scriptingLogger = nullptr;
        }
        inline static mem::Ref<log::Logger>& GetCoreLogger() { return s_coreLogger; }
        inline static mem::Ref<log::Logger>& GetGraphicsLogger() { return s_graphicsLogger; }
        inline static mem::Ref<log::Logger>& GetApplicationLogger() { return s_applicationLogger; }
        inline static mem::Ref<log::Logger>& GetScriptingLogger() { return s_scriptingLogger; }
    private:
        static inline mem::Ref<log::Logger> s_coreLogger = nullptr;
        static inline mem::Ref<log::Logger> s_graphicsLogger = nullptr;
        static inline mem::Ref<log::Logger> s_applicationLogger = nullptr;
        static inline mem::Ref<log::Logger> s_scriptingLogger = nullptr;
    };
}