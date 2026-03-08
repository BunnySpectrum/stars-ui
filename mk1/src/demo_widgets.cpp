#include "demo_widgets.h"
#include "theme.h"
#include "implot/implot.h"
#include <cmath>
#include <cstdio>

// ---------------------------------------------------------------------------
// Section header
// ---------------------------------------------------------------------------

void DrawSectionHeader(const char* text) {
    ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kTan));
    ImGui::SeparatorText(text);
    ImGui::PopStyleColor();
}

// ---------------------------------------------------------------------------
// Label / value pair
// ---------------------------------------------------------------------------

void DrawLabelValue(const char* label, const char* value) {
    ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kPurple));
    ImGui::Text("%s", label);
    ImGui::PopStyleColor();
    ImGui::SameLine(180.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kBeige));
    ImGui::Text("%s", value);
    ImGui::PopStyleColor();
}

// ---------------------------------------------------------------------------
// Label / value with threshold coloring
// ---------------------------------------------------------------------------

void DrawLabelValueThreshold(const char* label, const char* value, float numericVal, float warn, float crit) {
    ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kPurple));
    ImGui::Text("%s", label);
    ImGui::PopStyleColor();
    ImGui::SameLine(180.0f);

    ImU32 valColor = kBeige;
    if (numericVal >= crit)      valColor = kRed;
    else if (numericVal >= warn) valColor = kOrange;

    ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(valColor));
    ImGui::Text("%s", value);
    ImGui::PopStyleColor();
}

// ---------------------------------------------------------------------------
// Progress bar with threshold coloring
// ---------------------------------------------------------------------------

void DrawProgressBar(const char* label, float value, float warn, float crit) {
    ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kPurple));
    ImGui::Text("%s", label);
    ImGui::PopStyleColor();
    ImGui::SameLine(180.0f);

    ImU32 barColor = kBeige;
    if (value >= crit)      barColor = kRed;
    else if (value >= warn) barColor = kOrange;

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, U32ToVec4(barColor));
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    char overlay[32];
    std::snprintf(overlay, sizeof(overlay), "%.0f%%", value * 100.0f);
    ImGui::ProgressBar(value, ImVec2(0.0f, 0.0f), overlay);
    ImGui::PopStyleColor();
}

// ---------------------------------------------------------------------------
// Data table
// ---------------------------------------------------------------------------

void DrawDemoTable() {
    const int cols = 4;
    if (ImGui::BeginTable("##DemoTable", cols, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        // Headers
        ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kTan));
        ImGui::TableSetupColumn("SYSTEM");
        ImGui::TableSetupColumn("STATUS");
        ImGui::TableSetupColumn("POWER");
        ImGui::TableSetupColumn("UPTIME");
        ImGui::TableHeadersRow();
        ImGui::PopStyleColor();

        // Data rows
        struct Row { const char* sys; const char* status; const char* power; const char* uptime; };
        static const Row rows[] = {
            {"WARP CORE",    "ONLINE",  "98.2%",  "4821:03"},
            {"SHIELDS",      "STANDBY", "45.0%",  "4821:03"},
            {"PHASERS",      "OFFLINE", "0.0%",   "---"},
            {"LIFE SUPPORT", "ONLINE",  "100.0%", "4821:03"},
            {"SENSORS",      "ONLINE",  "87.5%",  "3201:15"},
        };

        ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kBeige));
        for (const auto& r : rows) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("%s", r.sys);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%s", r.status);
            ImGui::TableSetColumnIndex(2); ImGui::Text("%s", r.power);
            ImGui::TableSetColumnIndex(3); ImGui::Text("%s", r.uptime);
        }
        ImGui::PopStyleColor();

        ImGui::EndTable();
    }
}

// ---------------------------------------------------------------------------
// ImPlot chart
// ---------------------------------------------------------------------------

void DrawDemoChart() {
    static float t_data[256];
    static float y1[256];
    static float y2[256];

    float t = (float)ImGui::GetTime();
    for (int i = 0; i < 256; i++) {
        float x = (float)i / 256.0f * 4.0f * 3.14159f;
        t_data[i] = x;
        y1[i] = sinf(x + t) * 0.5f + 0.5f;
        y2[i] = cosf(x * 0.7f + t * 1.3f) * 0.4f + 0.5f;
    }

    ImPlot::PushStyleColor(ImPlotCol_Line, U32ToVec4(kOrange));
    if (ImPlot::BeginPlot("##DemoPlot", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("TIME", "VALUE", ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_None);
        ImPlot::SetupAxesLimits(0, 4.0 * 3.14159, -0.1, 1.1);

        ImPlot::PushStyleColor(ImPlotCol_Line, U32ToVec4(kOrange));
        ImPlot::PlotLine("PRIMARY", t_data, y1, 256);
        ImPlot::PopStyleColor();

        ImPlot::PushStyleColor(ImPlotCol_Line, U32ToVec4(kBlue));
        ImPlot::PlotLine("SECONDARY", t_data, y2, 256);
        ImPlot::PopStyleColor();

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

// ---------------------------------------------------------------------------
// Color palette swatches
// ---------------------------------------------------------------------------

void DrawColorPalette() {
    struct Swatch { const char* name; ImU32 color; };
    static const Swatch swatches[] = {
        {"ORANGE", kOrange},
        {"PURPLE", kPurple},
        {"BLUE",   kBlue},
        {"TAN",    kTan},
        {"RED",    kRed},
        {"BEIGE",  kBeige},
    };

    for (const auto& s : swatches) {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        float swW = 60.0f, swH = 24.0f;
        dl->AddRectFilled(pos, ImVec2(pos.x + swW, pos.y + swH), s.color, 4.0f);

        // Black text on colored swatch
        ImVec2 ts = ImGui::CalcTextSize(s.name);
        dl->AddText(ImVec2(pos.x + (swW - ts.x) * 0.5f, pos.y + (swH - ts.y) * 0.5f), IM_COL32(0, 0, 0, 255), s.name);

        ImGui::Dummy(ImVec2(swW, swH));
        ImGui::SameLine();
    }
    ImGui::NewLine();
}

// ---------------------------------------------------------------------------
// Scrolling log
// ---------------------------------------------------------------------------

void DrawDemoLog() {
    static const char* logLines[] = {
        "[4821:03:12] WARP CORE ALIGNMENT NOMINAL",
        "[4821:03:11] SHIELD FREQUENCY UPDATED TO 257.4 MHz",
        "[4821:03:10] SENSOR SWEEP SECTOR 7G COMPLETE",
        "[4821:03:09] LIFE SUPPORT RECYCLER CYCLE 4471",
        "[4821:03:08] NAVIGATIONAL DEFLECTOR RECALIBRATED",
        "[4821:03:07] COMM ARRAY SUBSPACE BAND LOCKED",
        "[4821:03:06] TRANSPORTER BUFFER PURGED",
        "[4821:03:05] ANTIMATTER CONTAINMENT FIELD STABLE",
        "[4821:03:04] INERTIAL DAMPENER CHECK PASSED",
        "[4821:03:03] PHASER BANK 2 MAINTENANCE SCHEDULED",
        "[4821:03:02] TURBOLIFT DIAGNOSTIC COMPLETE",
        "[4821:03:01] HOLODECK 3 SAFETY PROTOCOLS VERIFIED",
    };
    const int lineCount = sizeof(logLines) / sizeof(logLines[0]);

    ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kOrange));
    ImGui::BeginChild("##LogScroll", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_None);
    for (int i = 0; i < lineCount; i++) {
        ImGui::Text("%s", logLines[i]);
    }
    // Auto-scroll hint: show cursor time
    float t = (float)ImGui::GetTime();
    int sec = (int)t % 60;
    int min = ((int)t / 60) % 60;
    ImGui::Text("[4821:%02d:%02d] AWAITING INPUT...", min, sec);
    ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
    ImGui::PopStyleColor();
}
