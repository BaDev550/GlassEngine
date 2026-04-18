#pragma once

#include "GlassEngine/Utilities/Console.h"
#include "GlassEngine/Utilities/Time.h"
#include "GlassEngine/Utilities/Logger.h"
#include <imgui.h>

namespace ge::gui {
	class Console {
	public:
		void OnImGuiRender() {
			ImGui::SetNextWindowSize(ImVec2(600, 350), ImGuiCond_FirstUseEver);
			ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

			const float footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
			ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footerHeightToReserve), false, ImGuiWindowFlags_HorizontalScrollbar);
			const auto& messages = ge::Console::Get().GetLogMessages();

			if (_lastMessageCount != messages.size()) {
				_scrollToBottom = true;
				_lastMessageCount = messages.size();
			}

			for (const auto& message : messages) {
				ImGui::TextUnformatted(message.c_str());
			}

			if (_scrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
				ImGui::SetScrollHereY(1.0f);
				_scrollToBottom = false;
			}

			ImGui::EndChild();
			ImGui::Separator();

			bool reclaimFocus = false;
			ImGui::PushItemWidth(-1);

			ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue |
				ImGuiInputTextFlags_CallbackCompletion |
				ImGuiInputTextFlags_CallbackHistory |
				ImGuiInputTextFlags_CallbackEdit;

			if (ImGui::InputText("##input", _inputBuffer, IM_ARRAYSIZE(_inputBuffer), inputFlags, &TextEditCallbackStub, (void*)this)) {
				GEString command(_inputBuffer);
				if (!command.empty()) {
					GE_CORE_INFO("> {}", command);
					ge::Console::Get().ProcessCommand(command);
					_history.push_back(command);
				}
				_inputBuffer[0] = '\0';
				_historyPos = -1;
				_autoCompletePos = -1;
				reclaimFocus = true;
				_scrollToBottom = true;
			}
			ImGui::PopItemWidth();

			if (ImGui::IsItemFocused() && _inputBuffer[0] != '\0') {
				DrawAutoCompleteList(reclaimFocus);
			}

			ImGui::SetItemDefaultFocus();
			if (reclaimFocus) {
				ImGui::SetKeyboardFocusHere(-1);
			}
			ImGui::End();
		}
	private:
		void DrawAutoCompleteList(bool& reclaimFocus) {
			GEVector<GEString> matches = ge::Console::Get().GetCommandMatches(_inputBuffer);

			if (!matches.empty()) {
				ImVec2 pos = ImGui::GetItemRectMin();
				pos.y -= (matches.size() * ImGui::GetTextLineHeightWithSpacing()) + (ImGui::GetStyle().WindowPadding.y * 2);
				ImGui::SetNextWindowPos(pos, ImGuiCond_Always);

				ImGui::BeginTooltip();
				for (int i = 0; i < matches.size(); i++) {
					bool isSelected = (_autoCompletePos == i);
					if (ImGui::Selectable(matches[i].c_str(), isSelected)) {
						snprintf(_inputBuffer, sizeof(_inputBuffer), "%s", matches[i].c_str());
						_autoCompletePos = -1;
						reclaimFocus = true;
					}
				}
				ImGui::EndTooltip();
			}
		}

		static int TextEditCallbackStub(ImGuiInputTextCallbackData* data) {
			gui::Console* console = (gui::Console*)data->UserData;
			return console->TextEditCallback(data);
		}

		int TextEditCallback(ImGuiInputTextCallbackData* data) {
			if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit) {
				_autoCompletePos = -1;
				_historyPos = -1;
			}
			else if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion) {
				GEVector<GEString> matches = ge::Console::Get().GetCommandMatches(data->Buf);
				if (!matches.empty()) {
					int matchIndex = (_autoCompletePos >= 0 && _autoCompletePos < matches.size()) ? _autoCompletePos : 0;
					data->DeleteChars(0, data->BufTextLen);
					data->InsertChars(0, matches[matchIndex].c_str());
					_autoCompletePos = -1;
				}
			}
			else if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory) {
				GEVector<GEString> matches = ge::Console::Get().GetCommandMatches(data->Buf);
				if (data->BufTextLen > 0 && !matches.empty()) {
					if (data->EventKey == ImGuiKey_UpArrow) {
						_autoCompletePos--;
						if (_autoCompletePos < 0) _autoCompletePos = static_cast<int>(matches.size()) - 1;
					}
					else if (data->EventKey == ImGuiKey_DownArrow) {
						_autoCompletePos++;
						if (_autoCompletePos >= static_cast<int>(matches.size())) _autoCompletePos = 0;
					}
				}
				else {
					const int prevHistoryPos = _historyPos;

					if (data->EventKey == ImGuiKey_UpArrow) {
						if (_historyPos == -1) {
							_historyPos = static_cast<int>(_history.size()) - 1;
						}
						else if (_historyPos > 0) {
							_historyPos--;
						}
					}
					else if (data->EventKey == ImGuiKey_DownArrow) {
						if (_historyPos != -1) {
							if (++_historyPos >= static_cast<int>(_history.size())) {
								_historyPos = -1;
							}
						}
					}
					if (prevHistoryPos != _historyPos) {
						const char* historyStr = (_historyPos >= 0) ? _history[_historyPos].c_str() : "";
						data->DeleteChars(0, data->BufTextLen);
						data->InsertChars(0, historyStr);
					}
				}
			}
			return 0;
		}

		char _inputBuffer[256] = "";
		bool _scrollToBottom = false;
		size_t _lastMessageCount = 0;

		GEVector<GEString> _history;
		int _historyPos = -1;
		int _autoCompletePos = -1;
	};
}