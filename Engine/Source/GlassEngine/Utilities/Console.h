#pragma once
#include "GlassEngine/Core/Core.h"
#include <functional>

namespace ge {
	struct Command {
		GEString name;
		GEString usage;
		std::function<void(const GEVector<GEString>&)> action;
	};

	struct CommandList {
		GEString name;
		GEVector<Command> commands;
		void AddCommand(const GEString& name, std::function<void(const GEVector<GEString>&)> action, const GEString& usage) {
			commands.push_back({ name, usage, action });
		}
	};

#define GE_ADD_CONSOLE_COMMAND(category, name, action, ...) ge::Console::Get().AddCommand(category, name, action, __VA_ARGS__)
#define GE_EXECUTE_CONSOLE_COMMAND(command) ge::Console::Get().ProcessCommand(command)
	class Console {
	public:
		static GEString GetMessageFromArgs(const GEVector<GEString>& args) {
			GEString fullMessage = "";
			for (size_t i = 0; i < args.size(); ++i) {
				fullMessage += args[i];
				if (i != args.size() - 1) fullMessage += " ";
			}
			return fullMessage;
		}

		static void Init();
		static void Destroy();
		static Console& Get() {
			static Console instance;
			return instance;
		}

		void AddCommand(const GEString& category, const GEString& name, std::function<void(const GEVector<GEString>&)> action, const GEString& usage = "") {
			for (auto& list : _commandLists) {
				if (list.name == category) {
					list.AddCommand(name, action, usage);
					return;
				}
			}
			CommandList newList;
			newList.name = category;
			newList.AddCommand(name, action, usage);
			_commandLists.push_back(newList);
		}
		void Log(const GEString& message) {
			_consoleMessages.push_back(message);
		}

		void ClearLog() {
			_consoleMessages.clear();
		}

		const GEVector<GEString>& GetLogMessages() const { return _consoleMessages; }
		const GEVector<CommandList>& GetCommandLists() const { return _commandLists; }

		GEVector<GEString> GetCommandMatches(const GEString& partialInput);
		void ProcessCommand(const GEString& input);
	private:
		Console() = default;
		GEVector<CommandList> _commandLists;
		GEVector<GEString> _consoleMessages;
	};
}