#pragma once
#include "GlassEngine/Core/Core.h"
#include <functional>

namespace ge {
	struct Command {
		std::string name;
		std::function<void(const GEVector<std::string>&)> action;
	};

	struct CommandList {
		std::string name;
		GEVector<Command> commands;
		void AddCommand(const std::string& name, std::function<void(const GEVector<std::string>&)> action) {
			commands.push_back({ name, action });
		}
	};

	class Console {
	public:
		static void Init();
		static Console& Get() {
			static Console instance;
			return instance;
		}

		void AddCommand(const std::string& category, const std::string& name, std::function<void(const GEVector<std::string>&)> action) {
			for (auto& list : _commandLists) {
				if (list.name == category) {
					list.AddCommand(name, action);
					return;
				}
			}
			CommandList newList;
			newList.name = category;
			newList.AddCommand(name, action);
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