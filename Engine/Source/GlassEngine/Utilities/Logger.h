#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

#ifdef _DEBUG
#define GE_CORE_TRACE(...) ge::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define GE_CORE_INFO(...) ge::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define GE_CORE_WARN(...) ge::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define GE_CORE_ERROR(...) ge::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define GE_CORE_CRITICAL(...) ge::Logger::GetCoreLogger()->critical(__VA_ARGS__)

#define GE_APPLICATOIN_TRACE(...) ge::Logger::GetApplicationLogger()->trace(__VA_ARGS__)
#define GE_APPLICATOIN_INFO(...) ge::Logger::GetApplicationLogger()->info(__VA_ARGS__)
#define GE_APPLICATOIN_WARN(...) ge::Logger::GetApplicationLogger()->warn(__VA_ARGS__)
#define GE_APPLICATOIN_ERROR(...) ge::Logger::GetApplicationLogger()->error(__VA_ARGS__)

#define GE_GRAPCHICS_TRACE(...) ge::Logger::GetGraphicsLogger()->trace(__VA_ARGS__)
#define GE_GRAPCHICS_INFO(...) ge::Logger::GetGraphicsLogger()->info(__VA_ARGS__)
#define GE_GRAPCHICS_WARN(...) ge::Logger::GetGraphicsLogger()->warn(__VA_ARGS__)
#define GE_GRAPCHICS_ERROR(...) ge::Logger::GetGraphicsLogger()->error(__VA_ARGS__)
#define GE_GRAPCHICS_CRITICAL(...) ge::Logger::GetGraphicsLogger()->critical(__VA_ARGS__)

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
#define GE_APPLICATOIN_TRACE(...)
#define GE_APPLICATOIN_INFO(...)
#define GE_APPLICATOIN_WARN(...)
#define GE_APPLICATOIN_ERROR(...)
#define GE_GRAPCHICS_TRACE(...)
#define GE_GRAPCHICS_INFO(...)
#define GE_GRAPCHICS_WARN(...)
#define GE_GRAPCHICS_ERROR(...)
#define GE_GRAPCHICS_CRITICAL(...)
#define GE_SCRIPT_TRACE(...)
#define GE_SCRIPT_INFO(...)
#define GE_SCRIPT_WARN(...)
#define GE_SCRIPT_ERROR(...)
#endif

namespace ge {
	class Logger {
	public:
		static void Init() {
			auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
			spdlog::set_pattern("%^[%T] %n: %v%$");

			s_coreLogger = std::make_shared<spdlog::logger>("CORE", consoleSink);
			s_coreLogger->set_level(spdlog::level::trace);
			
			s_graphicsLogger = std::make_shared<spdlog::logger>("GRAPCHICS", consoleSink);
			s_graphicsLogger->set_level(spdlog::level::trace);
			
			s_applicationLogger = std::make_shared<spdlog::logger>("APPLICATION", consoleSink);
			s_applicationLogger->set_level(spdlog::level::trace);
			
			s_scriptingLogger = std::make_shared<spdlog::logger>("SCRIPT", consoleSink);
			s_scriptingLogger->set_level(spdlog::level::info);
		}
		static void Destroy() {
			s_coreLogger = nullptr;
			s_graphicsLogger = nullptr;
			s_applicationLogger = nullptr;
			s_scriptingLogger = nullptr;
		}
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_coreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetGraphicsLogger() { return s_graphicsLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetApplicationLogger() { return s_applicationLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetScriptingLogger() { return s_scriptingLogger; }
	private:
		static inline std::shared_ptr<spdlog::logger> s_coreLogger = nullptr;
		static inline std::shared_ptr<spdlog::logger> s_graphicsLogger = nullptr;
		static inline std::shared_ptr<spdlog::logger> s_applicationLogger = nullptr;
		static inline std::shared_ptr<spdlog::logger> s_scriptingLogger = nullptr;
	};
}