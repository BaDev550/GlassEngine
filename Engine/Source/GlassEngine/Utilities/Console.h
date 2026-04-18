#pragma once
#include "GlassEngine/Core/Core.h"
#include <functional>

namespace ge {
	struct Command {
		std::string name;
		std::string usage;
		std::function<void(const GEVector<std::string>&)> action;
	};

	struct CommandList {
		std::string name;
		GEVector<Command> commands;
		void AddCommand(const std::string& name, std::function<void(const GEVector<std::string>&)> action, const std::string& usage) {
			commands.push_back({ name, usage, action });
		}
	};

	class Console {
	public:
		static std::string GetMessageFromArgs(const GEVector<std::string>& args) {
			std::string fullMessage = "";
			for (size_t i = 0; i < args.size(); ++i) {
				fullMessage += args[i];
				if (i != args.size() - 1) fullMessage += " ";
			}
			return fullMessage;
		}

		static void Init();
		static Console& Get() {
			static Console instance;
			return instance;
		}

		void AddCommand(const std::string& category, const std::string& name, std::function<void(const GEVector<std::string>&)> action, const std::string& usage = "") {
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
		void Log(const std::string& message) {
			_consoleMessages.push_back(message);
		}

		void ClearLog() {
			_consoleMessages.clear();
		}

		const GEVector<std::string>& GetLogMessages() const { return _consoleMessages; }
		const GEVector<CommandList>& GetCommandLists() const { return _commandLists; }

		GEVector<std::string> GetCommandMatches(const std::string& partialInput);
		void ProcessCommand(const std::string& input);
	private:
		Console() = default;
		GEVector<CommandList> _commandLists;
		GEVector<std::string> _consoleMessages;
	};
}