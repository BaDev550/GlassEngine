#include "gepch.h"
#include "Console.h"
#include "GlassEngine/Utilities/Logger.h"

namespace ge {
	static std::string GetMessageFromArgs(const GEVector<std::string>& args) {
		std::string fullMessage = "";
		for (size_t i = 0; i < args.size(); ++i) {
			fullMessage += args[i];
			if (i != args.size() - 1) fullMessage += " ";
		}
		return fullMessage;
	}

	void Console::Init() {
		Get().AddCommand("console", "clear", [](const GEVector<std::string>& args) { Get().ClearLog(); });
	}

	GEVector<std::string> Console::GetCommandMatches(const std::string& partialInput) {
		GEVector<std::string> matches;
		std::string searchTarget = partialInput;
		size_t spacePos = partialInput.find(' ');
		if (spacePos != std::string::npos) {
			searchTarget = partialInput.substr(0, spacePos);
		}

		for (const auto& list : _commandLists) {
			for (const auto& cmd : list.commands) {
				std::string fullCmd = list.name + "." + cmd.name;
				if (fullCmd.find(searchTarget) == 0) {
					matches.push_back(fullCmd);
				}
			}
		}
		return matches;
	}

	void Console::ProcessCommand(const std::string& input) {
		if (input.empty()) return;
		std::istringstream iss(input);
		std::string commandName;
		iss >> commandName;

		GEVector<std::string> args;
		std::string arg;
		while (iss >> arg) {
			args.push_back(arg);
		}

		for (const auto& list : _commandLists) {
			for (const auto& cmd : list.commands) {
				if ((list.name + "." + cmd.name) == commandName) {
					if (!cmd.usage.empty() && args.empty()) {
						Log("Usage: " + cmd.usage);
						return;
					}
					cmd.action(args);
					return;
				}
			}
		}
		Log("Unknown command: " + commandName);
	}
}