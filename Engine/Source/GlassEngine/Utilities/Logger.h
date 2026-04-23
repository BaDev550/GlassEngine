#pragma once

#include "GlassEngine/Core/Memory.h"
#include "GlassEngine/Utilities/Time.h"
#include "GlassEngine/Utilities/Console.h"
#include <format>
#include <chrono>
#include <initializer_list>
#include <sstream>
#include <fstream>

#ifdef _DEBUG
#define GE_USE_APPLICATION_AND_STD_CONSOLE
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
#define GE_GLOBAL_SINK ge::Logger::GetGlobalSink()
#else 
#define GE_USE_APPLICATION_CONSOLE
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
#define GE_GRAPHICS_ERROR(...)
#define GE_GRAPHICS_CRITICAL(...)
#define GE_SCRIPT_TRACE(...)
#define GE_SCRIPT_INFO(...)
#define GE_SCRIPT_WARN(...)
#define GE_SCRIPT_ERROR(...)
// TODO (0x): fix this
#define GE_GLOBAL_SINK ge::Logger::GetGlobalSink()
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
#ifdef GE_USE_APPLICATION_CONSOLE
    #define GE_LOG(msg, type, name) ge::Console::Get().Log(GEString(GE_FORMAT_LOG_MESSAGE(msg, type, name).data(), GE_FORMAT_LOG_MESSAGE(msg, type, name).size()));
#elif defined(GE_USE_APPLICATION_AND_STD_CONSOLE)
#define GE_LOG(msg, type, name) \
    std::string formattedString = GE_FORMAT_LOG_MESSAGE(msg, type, name); \
    std::cout << formattedString << std::endl; \
    ge::Console::Get().Log(GEString(formattedString.data(), formattedString.size()));
#else
#define GE_LOG(msg, type, name) std::cout << GE_FORMAT_LOG_MESSAGE(msg, type, name) << std::endl;
#endif
#define GE_LOG_TO_SINK(msg, type, name, sink) if(sink) sink->Log(GE_FORMAT_LOG_MESSAGE(msg, type, name), type);

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
                GE_LOG(msg, LL_Info, _name);
				GE_LOG_TO_SINK(msg, LL_Info, _name, _sink);
            }

            template<typename... Args>
            void trace(std::format_string<Args...> fmt, Args&&... args) {
                auto msg = FormatMessage(fmt, std::forward<Args>(args)...);
                GE_LOG(msg, LL_Trace, _name);
                GE_LOG_TO_SINK(msg, LL_Trace, _name, _sink);
            }

            template<typename... Args>
            void warn(std::format_string<Args...> fmt, Args&&... args) {
                auto msg = FormatMessage(fmt, std::forward<Args>(args)...);
                GE_LOG(msg, LL_Warn, _name);
                GE_LOG_TO_SINK(msg, LL_Warn, _name, _sink);
            }

            template<typename... Args>
            void error(std::format_string<Args...> fmt, Args&&... args) {
                auto msg = FormatMessage(fmt, std::forward<Args>(args)...);
                GE_LOG(msg, LL_Error, _name);
                GE_LOG_TO_SINK(msg, LL_Error, _name, _sink);
            }

            template<typename... Args>
            void critical(std::format_string<Args...> fmt, Args&&... args) {
                auto msg = FormatMessage(fmt, std::forward<Args>(args)...);
                GE_LOG(msg, LL_Critical, _name);
                GE_LOG_TO_SINK(msg, LL_Critical, _name, _sink);
            }

        private:
            const char* _name;
			mem::Ref<Sink> _sink = nullptr;
        };
    }

    class Logger {
    public:
        static void Init() {
			s_globalSink = mem::Ref<log::Sink>::Create("GlobalSink", GEVector<log::logLevel>{ log::LL_Info, log::LL_Trace, log::LL_Warn, log::LL_Error, log::LL_Critical });
            s_coreLogger = mem::Ref<log::Logger>::Create("CORE", s_globalSink);
            s_graphicsLogger = mem::Ref<log::Logger>::Create("GRAPHICS", s_globalSink);
            s_applicationLogger = mem::Ref<log::Logger>::Create("APPLICATION", s_globalSink);
            s_scriptingLogger = mem::Ref<log::Logger>::Create("SCRIPT", s_globalSink);
        }
        static void Destroy() {
            s_coreLogger = nullptr;
            s_graphicsLogger = nullptr;
            s_applicationLogger = nullptr;
            s_scriptingLogger = nullptr;
			s_globalSink = nullptr;
        }
        inline static mem::Ref<log::Sink>& GetGlobalSink() { return s_globalSink; }
        inline static mem::Ref<log::Logger>& GetCoreLogger() { return s_coreLogger; }
        inline static mem::Ref<log::Logger>& GetGraphicsLogger() { return s_graphicsLogger; }
        inline static mem::Ref<log::Logger>& GetApplicationLogger() { return s_applicationLogger; }
        inline static mem::Ref<log::Logger>& GetScriptingLogger() { return s_scriptingLogger; }
    private:
		static inline mem::Ref<log::Sink> s_globalSink = nullptr;
        static inline mem::Ref<log::Logger> s_coreLogger = nullptr;
        static inline mem::Ref<log::Logger> s_graphicsLogger = nullptr;
        static inline mem::Ref<log::Logger> s_applicationLogger = nullptr;
        static inline mem::Ref<log::Logger> s_scriptingLogger = nullptr;
    };
}