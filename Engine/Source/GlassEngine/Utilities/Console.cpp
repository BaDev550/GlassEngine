#include "gepch.h"
#include "Console.h"
#include "GlassEngine/Utilities/Logger.h"

namespace ge {
	void Console::Init() {
		Get().AddCommand("console", "clear", [](const GEVector<GEString>& args) { Get().ClearLog(); });
	}
	void Console::Destroy() {
		Get().ClearLog();
		Get()._commandLists.clear();
	}

	GEVector<GEString> Console::GetCommandMatches(const GEString& partialInput) {
		GEVector<GEString> matches;
		GEString searchTarget = partialInput;
		size_t spacePos = partialInput.find(' ');
		if (spacePos != GEString::npos) {
			searchTarget = partialInput.substr(0, spacePos);
		}

		for (const auto& list : _commandLists) {
			for (const auto& cmd : list.commands) {
				GEString fullCmd = list.name + "." + cmd.name;
				if (fullCmd.find(searchTarget) == 0) {
					matches.push_back(fullCmd);
				}
			}
		}
		return matches;
	}

	void Console::ProcessCommand(const GEString& input) {
		if (input.empty()) return;
		std::istringstream iss(input);
		GEString commandName;
		iss >> commandName;

		GEVector<GEString> args;
		GEString arg;
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