#include "field_renderer.h"
#include "field_defs.h"
#include "field_store.h"
#include "svg_renderer.h"

#include <cstdio>
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

// ============================================================================
// SVG data-binding helpers
// ============================================================================

static ImU32 EvaluateThresholdColor(double value, const SvgBindingDef& b) {
    if (b.colorDir == ColorDir::LowIsWorse) {
        if (value < b.critThresh) return kRed;
        if (value < b.warnThresh) return kOrange;
    } else { // HighIsWorse
        if (value >= b.critThresh) return kRed;
        if (value >= b.warnThresh) return kOrange;
    }
    return b.normalColor;
}

void ApplySvgBindingColors(SvgRenderer& svg, const char* viewName) {
    for (int i = 0; i < kNumSvgBindings; i++) {
        const SvgBindingDef& b = kSvgBindings[i];
        if (std::strcmp(b.view, viewName) != 0) continue;
        if (b.normalColor == 0) continue;

        double value = g_fields.Get(b.field);
        ImU32 color = EvaluateThresholdColor(value, b);
        svg.SetShapeColor(b.shapeId, color);
    }
}

void DrawSvgBindingLabels(const SvgRenderer& svg, const char* viewName,
                          ImVec2 origin, ImVec2 size) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    constexpr float pad = 4.0f;

    for (int i = 0; i < kNumSvgBindings; i++) {
        const SvgBindingDef& b = kSvgBindings[i];
        if (std::strcmp(b.view, viewName) != 0) continue;

        ImVec4 r = svg.GetShapeBounds(b.shapeId, origin, size);
        if (r.z <= 0) continue;

        // Line 1: label
        const char* labelText = b.label;
        if (!labelText) {
            for (int f = 0; f < kNumFields; f++) {
                if (kFields[f].id == b.field) { labelText = kFields[f].label; break; }
            }
        }
        if (!labelText) labelText = "???";

        // Line 2: value
        char valueBuf[64];
        if (b.valueFmt) {
            std::snprintf(valueBuf, sizeof(valueBuf), b.valueFmt, g_fields.Get(b.field));
        } else {
            std::snprintf(valueBuf, sizeof(valueBuf), "%s", g_fields.GetString(b.field));
        }

        ImVec2 labelSize = ImGui::CalcTextSize(labelText);
        ImVec2 valueSize = ImGui::CalcTextSize(valueBuf);
        float lineH = ImGui::GetTextLineHeight();
        float blockW = (labelSize.x > valueSize.x) ? labelSize.x : valueSize.x;
        float blockH = lineH * 2.0f + 2.0f; // two lines + small gap

        // Shape center
        float cx = r.x + r.z * 0.5f;
        float cy = r.y + r.w * 0.5f;

        // Position the text block based on anchor
        float bx, by;
        switch (b.anchor) {
        case LabelAnchor::Center:
            bx = cx - blockW * 0.5f;
            by = cy - blockH * 0.5f;
            break;
        case LabelAnchor::Above:
            bx = cx - blockW * 0.5f;
            by = r.y - blockH - pad;
            break;
        case LabelAnchor::Below:
            bx = cx - blockW * 0.5f;
            by = r.y + r.w + pad;
            break;
        case LabelAnchor::Left:
            bx = r.x - blockW - pad;
            by = cy - blockH * 0.5f;
            break;
        case LabelAnchor::Right:
            bx = r.x + r.z + pad;
            by = cy - blockH * 0.5f;
            break;
        }

        // Dark background rect
        ImU32 bgColor = IM_COL32(0, 0, 0, 180);
        dl->AddRectFilled(ImVec2(bx - pad, by - pad),
                          ImVec2(bx + blockW + pad, by + blockH + pad),
                          bgColor, 2.0f);

        // Label text (centered within block)
        float labelX = bx + (blockW - labelSize.x) * 0.5f;
        dl->AddText(ImVec2(labelX, by), kBeige, labelText);

        // Value text (centered within block)
        float valueX = bx + (blockW - valueSize.x) * 0.5f;
        dl->AddText(ImVec2(valueX, by + lineH + 2.0f), kBeige, valueBuf);
    }
}
