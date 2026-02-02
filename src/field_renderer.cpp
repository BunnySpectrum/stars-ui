#include "field_renderer.h"
#include "field_defs.h"
#include "field_store.h"

#include <cstring>
#include <vector>

// Estimate the height needed to render a column of fields.
static float EstimateColumnHeight(const std::vector<const FieldDef*>& fields) {
    float lineH = ImGui::GetTextLineHeightWithSpacing();
    float secH  = ImGui::GetFrameHeight() + 4.0f; // SeparatorText height
    int nSections = 0;
    const char* lastSec = nullptr;
    for (auto* f : fields) {
        if (!lastSec || std::strcmp(f->section, lastSec) != 0) {
            nSections++;
            lastSec = f->section;
        }
    }
    return nSections * (secH + 8.0f) + (int)fields.size() * lineH + 16.0f;
}

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
    // Collect fields for this view, split by column index
    int maxCol = 0;
    std::vector<const FieldDef*> cols[8]; // up to 8 columns
    for (int i = 0; i < kNumFields; i++) {
        if (std::strcmp(kFields[i].view, viewName) != 0) continue;
        int c = kFields[i].column;
        if (c < 0) c = 0;
        if (c > 7) c = 7;
        cols[c].push_back(&kFields[i]);
        if (c > maxCol) maxCol = c;
    }

    int nCols = maxCol + 1;

    // Find the tallest column to use as the child height
    float maxH = 0.0f;
    for (int c = 0; c <= maxCol; c++) {
        float h = EstimateColumnHeight(cols[c]);
        if (h > maxH) maxH = h;
    }

    if (nCols <= 1) {
        ImGui::BeginChild("##Col0", ImVec2(0, maxH), false);
        DrawFieldColumn(cols[0]);
        ImGui::EndChild();
    } else {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float gap = 8.0f;
        float colW = (avail.x - gap * (nCols - 1)) / (float)nCols;

        for (int c = 0; c <= maxCol; c++) {
            if (c > 0) ImGui::SameLine(0, gap);
            char id[16];
            snprintf(id, sizeof(id), "##Col%d", c);
            ImGui::BeginChild(id, ImVec2(colW, maxH), false);
            DrawFieldColumn(cols[c]);
            ImGui::EndChild();
        }
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
