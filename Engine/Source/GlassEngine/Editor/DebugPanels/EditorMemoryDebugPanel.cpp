#include "gepch.h"
#include "GlassEngine/Renderer/RenderAPI.h"
#include "GlassEngine/Renderer/Renderer.h"
#include "GlassEngine/Utilities/MemoryProfiler.h"
#include "GlassEngine/Memory/Allocator.h"
#include "EditorMemoryDebugPanel.h"

#include <imgui.h>

namespace ge::editor {
    std::string FormatBytes(size_t bytes) {
        if (bytes < 1024) return std::to_string(bytes) + " B";
        if (bytes < 1024 * 1024) return std::to_string(bytes / 1024) + " KB";
        return std::to_string(bytes / (1024 * 1024)) + " MB";
    }

    std::string ResolveStackTrace(void* const* stack, int frameCount) {
        std::ostringstream ss;
#ifdef _WIN32
        HANDLE process = GetCurrentProcess();
        SymInitialize(process, NULL, TRUE);

        char symbolBuffer[sizeof(SYMBOL_INFO) + 256];
        SYMBOL_INFO* symbol = (SYMBOL_INFO*)symbolBuffer;
        symbol->MaxNameLen = 255;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        for (int i = 0; i < frameCount; i++) {
            if (stack[i] == nullptr) continue;
            if (SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol)) {
                ss << "[" << i << "] " << symbol->Name << "\n";
            }
        }
#else
        char** symbols = backtrace_symbols(stack, frameCount);
        if (symbols) {
            for (int i = 0; i < frameCount; i++) {
                ss << "[" << i << "] " << symbols[i] << "\n";
            }
            free(symbols);
        }
#endif
        return ss.str();
    }

	void EditorMemoryDebugPanel::Draw()
	{
        auto& profiler = ge::MemoryProfiler::Get();

        if (ImGui::Begin("Memory Profiler")) {

            ImGui::Text("Current Memory Usage: %s", FormatBytes(profiler.GetTotalAllocated()).c_str());
            ImGui::Text("Active Allocations: %zu", profiler.GetActiveAllocations().size());
            ImGui::Separator();

            if (ImGui::CollapsingHeader("Memory by Category", ImGuiTreeNodeFlags_DefaultOpen)) {
                for (const auto& [category, size] : profiler.GetCategoryUsage()) {
                    if (size == 0) continue;

                    float fraction = (float)size / (float)profiler.GetTotalAllocated();
                    std::string overlay = category + ": " + FormatBytes(size);
                    ImGui::ProgressBar(fraction, ImVec2(-1, 0), overlay.c_str());
                }
            }

            if (ImGui::CollapsingHeader("Live Allocations")) {
                std::vector<std::pair<void*, ge::AllocRecord>> allocs(
                    profiler.GetActiveAllocations().begin(),
                    profiler.GetActiveAllocations().end()
                );

                std::sort(allocs.begin(), allocs.end(), [](const auto& a, const auto& b) { return a.second.size > b.second.size; });

                if (ImGui::BeginTable("AllocTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                    ImGui::TableSetupColumn("Address");
                    ImGui::TableSetupColumn("Size");
                    ImGui::TableSetupColumn("Category");
                    ImGui::TableSetupColumn("Origin");
                    ImGui::TableHeadersRow();

                    for (const auto& [ptr, record] : allocs) {
                        ImGui::TableNextRow();

                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%p", ptr);

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%s", FormatBytes(record.size).c_str());

                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%s", record.category);

                        ImGui::TableSetColumnIndex(3);
                        ImGui::TextDisabled("[Hover for Stack]");

                        if (ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            std::string resolvedTrace = ResolveStackTrace(record.stackTrace, record.frameCount);
                            if (resolvedTrace.empty()) {
                                ImGui::TextUnformatted("No stack trace available.");
                            }
                            else {
                                ImGui::TextUnformatted(resolvedTrace.c_str());
                            }
                            ImGui::EndTooltip();
                        }
                    }
                    ImGui::EndTable();
                }
            }
        }
        ImGui::End();
    }
}