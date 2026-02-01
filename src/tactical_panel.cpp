#include "tactical_panel.h"

#include <cmath>

const char* TacticalPanel::GetTitle() const {
    return "LCARS TACTICAL DISPLAY 47";
}

void TacticalPanel::DrawUpper() {
    ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kTan));
    ImGui::SeparatorText("AUXILIARY DISPLAY");
    ImGui::PopStyleColor();
    ImGui::TextColored(U32ToVec4(kBeige), "AWAITING INPUT...");
}

void TacticalPanel::DrawLower() {
    // --- System Status Panel ---
    ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kTan));
    ImGui::SeparatorText("SYSTEM STATUS");
    ImGui::PopStyleColor();

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, 200.0f);

    ImGui::TextColored(U32ToVec4(kPurple), "WARP CORE");
    ImGui::TextColored(U32ToVec4(kPurple), "SHIELDS");
    ImGui::TextColored(U32ToVec4(kPurple), "HULL INTEGRITY");
    ImGui::TextColored(U32ToVec4(kPurple), "LIFE SUPPORT");

    ImGui::NextColumn();

    ImGui::TextColored(U32ToVec4(kBeige), "ONLINE  -  OUTPUT 98.7%%");
    ImGui::TextColored(U32ToVec4(kBeige), "ACTIVE  -  STRENGTH 100%%");
    ImGui::TextColored(U32ToVec4(kBeige), "NOMINAL -  97.3%%");
    ImGui::TextColored(U32ToVec4(kBeige), "NOMINAL -  ALL DECKS");

    ImGui::Columns(1);
    ImGui::Spacing();
    ImGui::Spacing();

    // --- Sensor Telemetry Panel ---
    ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kTan));
    ImGui::SeparatorText("SENSOR TELEMETRY");
    ImGui::PopStyleColor();

    // Animated sine/cosine data
    float t = (float)ImGui::GetTime();
    static const int N = 256;
    static float xs[N], ys1[N], ys2[N];
    for (int i = 0; i < N; i++) {
        xs[i]  = (float)i / (float)(N - 1) * 10.0f;
        ys1[i] = sinf(xs[i] + t) * 0.5f + 0.5f * sinf(xs[i] * 2.3f + t * 1.7f);
        ys2[i] = cosf(xs[i] * 0.7f + t * 0.8f) * 0.8f;
    }

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float plotW = avail.x;
    float plotH = avail.y;
    if (plotH < 100.0f) plotH = 100.0f;

    ImPlot::PushStyleColor(ImPlotCol_Line, U32ToVec4(kOrange));
    if (ImPlot::BeginPlot("##Telemetry", ImVec2(plotW, plotH))) {
        ImPlot::SetupAxes("FREQUENCY", "AMPLITUDE");
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, 10, ImPlotCond_Once);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -1.5, 1.5, ImPlotCond_Once);

        ImPlot::PushStyleColor(ImPlotCol_Line, U32ToVec4(kOrange));
        ImPlot::PlotLine("Subspace Band", xs, ys1, N);
        ImPlot::PopStyleColor();

        ImPlot::PushStyleColor(ImPlotCol_Line, U32ToVec4(kBlue));
        ImPlot::PlotLine("EM Spectrum", xs, ys2, N);
        ImPlot::PopStyleColor();

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}
