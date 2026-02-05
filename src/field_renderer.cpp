#include "field_renderer.h"
#include "svg_renderer.h"

#include <cmath>

// Look up threshold definition for a field (returns nullptr if none)
static const FieldThresholdDef* FindFieldThreshold(FieldId id) {
    for (int i = 0; i < kNumFieldThresholds; i++) {
        if (kFieldThresholds[i].field == id)
            return &kFieldThresholds[i];
    }
    return nullptr;
}

// Evaluate color for a field based on threshold (returns 0 if no threshold defined)
// Non-static because it's used by template functions in the header
ImU32 EvaluateFieldColor(FieldId id) {
    const FieldThresholdDef* t = FindFieldThreshold(id);
    if (!t) return 0;

    double value = g_fields.Get(id);
    if (t->colorDir == ColorDir::LowIsWorse) {
        if (value < t->critThresh) return kRed;
        if (value < t->warnThresh) return kOrange;
    } else { // HighIsWorse
        if (value >= t->critThresh) return kRed;
        if (value >= t->warnThresh) return kOrange;
    }
    return 0; // normal — use default color
}

// Template implementations are in field_renderer.h

void DrawFieldsForView(ViewId viewId) {
    // Collect fields for this view, split by column index
    int maxCol = 0;
    std::vector<const FieldDef*> cols[8];
    for (int i = 0; i < kNumFields; i++) {
        if (kFields[i].view != viewId) continue;
        int c = kFields[i].column;
        if (c < 0) c = 0;
        if (c > 7) c = 7;
        cols[c].push_back(&kFields[i]);
        if (c > maxCol) maxCol = c;
    }

    int nCols = maxCol + 1;

    float maxH = 0.0f;
    for (int c = 0; c <= maxCol; c++) {
        float h = detail::EstimateColumnHeight(cols[c]);
        if (h > maxH) maxH = h;
    }

    if (nCols <= 1) {
        ImGui::BeginChild("##Col0", ImVec2(0, maxH), false);
        detail::DrawFieldColumn(cols[0]);
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
            detail::DrawFieldColumn(cols[c]);
            ImGui::EndChild();
        }
    }
}

void DrawGraphsForView(ViewId viewId) {
    for (int g = 0; g < kNumGraphs; g++) {
        if (kGraphs[g].view != viewId) continue;

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

        if (ImPlot::BeginPlot(GraphIdToPlotId(gd.id), ImVec2(plotW, plotH))) {
            ImPlot::SetupAxes(gd.xLabel, gd.yLabel);
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, kGraphXMax, ImPlotCond_Once);
            ImPlot::SetupAxisLimits(ImAxis_Y1, gd.yMin, gd.yMax, ImPlotCond_Once);

            for (int l = 0; l < 2; l++) {
                const char* label = GraphLineIdToLabel(gd.lines[l].id);
                if (!label) continue;
                ImPlot::PushStyleColor(ImPlotCol_Line, U32ToVec4(gd.lines[l].color));
                ImPlot::PlotLine(label, buf.xs, buf.ys[l], kGraphSamples);
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

void ApplySvgBindingColors(SvgRenderer& svg, ViewId viewId) {
    for (int i = 0; i < kNumSvgBindings; i++) {
        const SvgBindingDef& b = kSvgBindings[i];
        if (b.view != viewId) continue;
        if (b.normalColor == 0) continue;

        double value = g_fields.Get(b.field);
        ImU32 color = EvaluateThresholdColor(value, b);
        svg.SetShapeColor(ShapeIdToString(b.shapeId), color);
    }
}

void ApplySvgBindingColors(SvgRenderer& svg, std::span<const SvgBindingDef> bindings) {
    for (const auto& b : bindings) {
        if (b.normalColor == 0) continue;

        double value = g_fields.Get(b.field);
        ImU32 color = EvaluateThresholdColor(value, b);
        svg.SetShapeColor(ShapeIdToString(b.shapeId), color);
    }
}

void ApplyDefaultColors(SvgRenderer& svg, std::span<const ShapeColorDef> colors) {
    for (const auto& c : colors) {
        svg.SetShapeColor(ShapeIdToString(c.shapeId), c.color);
    }
}

void ApplyShapeAnimations(SvgRenderer& svg, std::span<const ShapeAnimDef> animations, float time) {
    for (const auto& anim : animations) {
        ImU32 color;

        switch (anim.type) {
        case AnimType::Blink: {
            float period = 1.0f / anim.frequency;
            float t = fmodf(time, period);
            color = (t < period * 0.5f) ? anim.color1 : anim.color2;
            break;
        }
        case AnimType::AlphaPulse: {
            float pulse = (sinf(time * anim.frequency) + 1.0f) * 0.5f;
            int alpha = anim.minAlpha + (int)(pulse * (255 - anim.minAlpha));
            color = (anim.color1 & 0x00FFFFFF) | ((ImU32)alpha << 24);
            break;
        }
        }

        svg.SetShapeColor(ShapeIdToString(anim.shapeId), color);
    }
}

void DrawSvgBindingLabels(const SvgRenderer& svg, ViewId viewId,
                          ImVec2 origin, ImVec2 size) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    constexpr float pad = 4.0f;

    for (int i = 0; i < kNumSvgBindings; i++) {
        const SvgBindingDef& b = kSvgBindings[i];
        if (b.view != viewId) continue;

        ImVec4 r = svg.GetShapeBounds(ShapeIdToString(b.shapeId), origin, size);
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

void DrawSvgBindingLabels(const SvgRenderer& svg, std::span<const SvgBindingDef> bindings,
                          ImVec2 origin, ImVec2 size) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    constexpr float pad = 4.0f;

    for (const auto& b : bindings) {
        ImVec4 r = svg.GetShapeBounds(ShapeIdToString(b.shapeId), origin, size);
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
        float blockH = lineH * 2.0f + 2.0f;

        float cx = r.x + r.z * 0.5f;
        float cy = r.y + r.w * 0.5f;

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

        ImU32 bgColor = IM_COL32(0, 0, 0, 180);
        dl->AddRectFilled(ImVec2(bx - pad, by - pad),
                          ImVec2(bx + blockW + pad, by + blockH + pad),
                          bgColor, 2.0f);

        float labelX = bx + (blockW - labelSize.x) * 0.5f;
        dl->AddText(ImVec2(labelX, by), kBeige, labelText);

        float valueX = bx + (blockW - valueSize.x) * 0.5f;
        dl->AddText(ImVec2(valueX, by + lineH + 2.0f), kBeige, valueBuf);
    }
}

// ============================================================================
// Animation helpers for data-driven SVG views
// ============================================================================

void ApplyShapeAnimations(SvgRenderer& svg, const ViewDef& view, float time) {
    for (int i = 0; i < view.animationCount; i++) {
        const ShapeAnimDef& anim = view.animations[i];
        ImU32 color;

        switch (anim.type) {
        case AnimType::Blink: {
            // Toggle between color1 and color2 based on frequency
            float period = 1.0f / anim.frequency;
            float t = fmodf(time, period);
            color = (t < period * 0.5f) ? anim.color1 : anim.color2;
            break;
        }
        case AnimType::AlphaPulse: {
            // Sin-wave alpha variation from minAlpha to 255
            float pulse = (sinf(time * anim.frequency) + 1.0f) * 0.5f;
            int alpha = anim.minAlpha + (int)(pulse * (255 - anim.minAlpha));
            // Extract RGB from color1, apply computed alpha
            color = (anim.color1 & 0x00FFFFFF) | ((ImU32)alpha << 24);
            break;
        }
        }

        svg.SetShapeColor(ShapeIdToString(anim.shapeId), color);
    }
}

void DrawGlowOverlay(const SvgRenderer& svg, const GlowOverlayDef& glow,
                     ImVec2 origin, ImVec2 size, float time) {
    if (glow.shapeId == ShapeId::None) return;

    ImVec4 r = svg.GetShapeBounds(ShapeIdToString(glow.shapeId), origin, size);
    if (r.z <= 0) return;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float pulse = (sinf(time * glow.frequency) + 1.0f) * 0.5f;
    int alpha = (int)(pulse * glow.maxAlpha);
    ImU32 glowColor = (glow.color & 0x00FFFFFF) | ((ImU32)alpha << 24);

    float cx = r.x + r.z * 0.5f;
    float cy = r.y + r.w * 0.5f;
    dl->AddEllipseFilled(ImVec2(cx, cy), ImVec2(r.z * 0.5f, r.w * 0.5f), glowColor);
}

