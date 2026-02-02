#include "field_renderer.h"
#include "field_defs.h"
#include "field_store.h"

#include <cstring>
#include <vector>

// Draw a group of fields as section-header + label/value columns.
static void DrawFieldColumn(const std::vector<const FieldDef*>& fields) {
    int i = 0;
    while (i < (int)fields.size()) {
        const char* section = fields[i]->section;
        int secStart = i;

        // Find end of this section
        while (i < (int)fields.size() && std::strcmp(fields[i]->section, section) == 0)
            i++;
        int secEnd = i;

        // Section header
        ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kTan));
        ImGui::SeparatorText(section);
        ImGui::PopStyleColor();

        // Label-value pairs
        ImGui::Columns(2, nullptr, false);
        ImGui::SetColumnWidth(0, 200.0f);

        for (int j = secStart; j < secEnd; j++)
            ImGui::TextColored(U32ToVec4(kPurple), "%s", fields[j]->label);

        ImGui::NextColumn();

        for (int j = secStart; j < secEnd; j++)
            ImGui::TextColored(U32ToVec4(kBeige), "%s", g_fields.GetString(fields[j]->id));

        ImGui::Columns(1);
        ImGui::Spacing();
        ImGui::Spacing();
    }
}

void DrawFieldsForView(const char* viewName) {
    // Collect fields for this view, split by column
    std::vector<const FieldDef*> col0, col1;
    for (int i = 0; i < kNumFields; i++) {
        if (std::strcmp(kFields[i].view, viewName) != 0) continue;
        if (kFields[i].column == 1)
            col1.push_back(&kFields[i]);
        else
            col0.push_back(&kFields[i]);
    }

    if (!col1.empty()) {
        // Side-by-side layout
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float halfW = avail.x * 0.5f - 4.0f;

        ImGui::BeginChild("##Col0", ImVec2(halfW, 0), false);
        DrawFieldColumn(col0);
        ImGui::EndChild();

        ImGui::SameLine(0, 8.0f);

        ImGui::BeginChild("##Col1", ImVec2(halfW, 0), false);
        DrawFieldColumn(col1);
        ImGui::EndChild();
    } else {
        DrawFieldColumn(col0);
    }
}

void DrawGraphsForView(const char* viewName) {
    for (int g = 0; g < kNumGraphs; g++) {
        if (std::strcmp(kGraphs[g].view, viewName) != 0) continue;

        const GraphDef& gd  = kGraphs[g];
        const GraphBuffer& buf = g_graphBufs[g];

        if (gd.section) {
            ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kTan));
            ImGui::SeparatorText(gd.section);
            ImGui::PopStyleColor();
        }

        ImVec2 avail = ImGui::GetContentRegionAvail();
        float plotW = avail.x;
        float plotH = avail.y;
        if (plotH < 100.0f) plotH = 100.0f;

        if (ImPlot::BeginPlot(gd.plotId, ImVec2(plotW, plotH))) {
            ImPlot::SetupAxes(gd.xLabel, gd.yLabel);
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, kGraphXMax, ImPlotCond_Once);
            ImPlot::SetupAxisLimits(ImAxis_Y1, gd.yMin, gd.yMax, ImPlotCond_Once);

            for (int l = 0; l < 2; l++) {
                if (!gd.lines[l].label) continue;
                ImPlot::PushStyleColor(ImPlotCol_Line, U32ToVec4(gd.lines[l].color));
                ImPlot::PlotLine(gd.lines[l].label, buf.xs, buf.ys[l], kGraphSamples);
                ImPlot::PopStyleColor();
            }

            ImPlot::EndPlot();
        }
    }
}
