#pragma once

#include "imgui.h"
#include "implot.h"
#include "views/view_common.h"
#include "field_defs.h"
#include "field_store.h"

#include <cstdio>
#include <cstring>
#include <vector>

// Generic rendering for view content.
// Call DrawViewContent() from a PanelView's drawContent lambda.

// ============================================================================
// Template implementations for generic view rendering
// ============================================================================

namespace detail {

template<typename FieldDefT>
inline float EstimateColumnHeight(const std::vector<const FieldDefT*>& fields) {
    float lineH = ImGui::GetTextLineHeightWithSpacing();
    float secH  = ImGui::GetFrameHeight() + 4.0f;
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

template<typename FieldDefT>
inline void DrawFieldColumn(const std::vector<const FieldDefT*>& fields) {
    int i = 0;
    while (i < (int)fields.size()) {
        const char* section = fields[i]->section;
        int secStart = i;

        while (i < (int)fields.size() && std::strcmp(fields[i]->section, section) == 0)
            i++;
        int secEnd = i;

        ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kTan));
        ImGui::SeparatorText(section);
        ImGui::PopStyleColor();

        ImGui::Columns(2, nullptr, false);
        ImGui::SetColumnWidth(0, 200.0f);

        for (int j = secStart; j < secEnd; j++)
            ImGui::TextColored(U32ToVec4(kPurple), "%s", fields[j]->label);

        ImGui::NextColumn();

        for (int j = secStart; j < secEnd; j++) {
            const auto* f = fields[j];
            FieldId fid = f->GetFieldId();
            ImU32 color = kBeige;
            if (f->colorDir != ColorDir::None) {
                double value = g_fields.Get(fid);
                if (f->colorDir == ColorDir::LowIsWorse) {
                    if (value < f->critThresh) color = kRed;
                    else if (value < f->warnThresh) color = kOrange;
                } else { // HighIsWorse
                    if (value >= f->critThresh) color = kRed;
                    else if (value >= f->warnThresh) color = kOrange;
                }
            }
            ImGui::TextColored(U32ToVec4(color), "%s", g_fields.GetString(fid));
        }

        ImGui::Columns(1);
        ImGui::Spacing();
        ImGui::Spacing();
    }
}

} // namespace detail

// Draw fields from a span, split into columns
template<typename FieldDefT>
inline void DrawFieldsFromTable(std::span<const FieldDefT> fields) {
    int maxCol = 0;
    std::vector<const FieldDefT*> cols[8];
    for (const auto& f : fields) {
        int c = f.column;
        if (c < 0) c = 0;
        if (c > 7) c = 7;
        cols[c].push_back(&f);
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

// Draw graphs from a span
template<typename GraphDefT>
inline void DrawGraphsFromTable(std::span<const GraphDefT> graphs) {
    for (const auto& gd : graphs) {
        const GraphBuffer& buf = g_graphBufs[static_cast<int>(gd.id)];

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

// Generic view content drawing - uses duck typing for fields, graphs, and SVG
template<typename ContentT>
inline void DrawViewContent(ContentT& content) {
    // 1. Draw fields if present and non-empty
    if constexpr (requires { content.fields; }) {
        if (!content.fields.empty())
            DrawFieldsFromTable(content.fields);
    }

    // 2. Draw graphs if present and non-empty
    if constexpr (requires { content.graphs; }) {
        if (!content.graphs.empty())
            DrawGraphsFromTable(content.graphs);
    }

    // 3. Draw SVG if renderer is set
    if constexpr (requires { content.svg; content.svgPath; }) {
        if (content.svg) {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImVec2 cursor = ImGui::GetCursorScreenPos();
            float time = (float)ImGui::GetTime();

            // Apply bindings if present
            if constexpr (requires { content.svgBindings; }) {
                ApplySvgBindingColors(*content.svg, content.svgBindings);
            }

            // Apply animations if present
            if constexpr (requires { content.animations; }) {
                ApplyShapeAnimations(*content.svg, content.animations, time);
            }

            // Draw the SVG
            content.svg->Draw(cursor, avail, content.strokeColor, content.strokeWidth);

            // Draw labels if present (requires bindingFields for label lookup)
            if constexpr (requires { content.svgBindings; content.bindingFields; }) {
                DrawSvgBindingLabels(*content.svg, content.svgBindings, content.bindingFields, cursor, avail);
            }

            // Draw glow overlay if present
            if constexpr (requires { content.glowOverlay; }) {
                DrawGlowOverlay(*content.svg, content.glowOverlay, cursor, avail, time);
            }

            ImGui::Dummy(avail);
        }
    }
}

// SVG data-binding helpers — drive shape colors and overlay labels from field data.
class SvgRenderer;
struct GlowOverlayDef;
struct SvgBindingDef;
struct ShapeAnimDef;
struct ShapeColorDef;

// Span-based versions (for views with per-view binding data)
void ApplySvgBindingColors(SvgRenderer& svg, std::span<const SvgBindingDef> bindings);

// Templated version that looks up labels from per-view field definitions
template<typename FieldDefT>
void DrawSvgBindingLabels(const SvgRenderer& svg, std::span<const SvgBindingDef> bindings,
                          std::span<const FieldDefT> fields, ImVec2 origin, ImVec2 size);

// Template versions using duck typing - call def.GetShapeString()
template<typename ColorDefT>
inline void ApplyDefaultColors(SvgRenderer& svg, std::span<const ColorDefT> colors);

template<typename AnimDefT>
inline void ApplyShapeAnimations(SvgRenderer& svg, std::span<const AnimDefT> animations, float time);

// Non-template overloads for backward compatibility with ShapeColorDef/ShapeAnimDef
void ApplyDefaultColors(SvgRenderer& svg, std::span<const ShapeColorDef> colors);
void ApplyShapeAnimations(SvgRenderer& svg, std::span<const ShapeAnimDef> animations, float time);
void DrawGlowOverlay(const SvgRenderer& svg, const GlowOverlayDef& glow,
                     ImVec2 origin, ImVec2 size, float time);

// Template version for duck-typed glow overlay
template<typename GlowDefT>
inline void DrawGlowOverlay(const SvgRenderer& svg, const GlowDefT& glow,
                            ImVec2 origin, ImVec2 size, float time);

// ============================================================================
// Template implementations for SVG rendering (duck-typed)
// ============================================================================

#include "svg_renderer.h"
#include <cmath>

template<typename ColorDefT>
inline void ApplyDefaultColors(SvgRenderer& svg, std::span<const ColorDefT> colors) {
    for (const auto& c : colors) {
        svg.SetShapeColor(c.GetShapeString(), c.color);
    }
}

template<typename AnimDefT>
inline void ApplyShapeAnimations(SvgRenderer& svg, std::span<const AnimDefT> animations, float time) {
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

        svg.SetShapeColor(anim.GetShapeString(), color);
    }
}

template<typename GlowDefT>
inline void DrawGlowOverlay(const SvgRenderer& svg, const GlowDefT& glow,
                            ImVec2 origin, ImVec2 size, float time) {
    const char* shapeStr = glow.GetShapeString();
    if (!shapeStr) return;

    ImVec4 r = svg.GetShapeBounds(shapeStr, origin, size);
    if (r.z <= 0) return;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    float pulse = (sinf(time * glow.frequency) + 1.0f) * 0.5f;
    int alpha = (int)(pulse * glow.maxAlpha);
    ImU32 glowColor = (glow.color & 0x00FFFFFF) | ((ImU32)alpha << 24);

    float cx = r.x + r.z * 0.5f;
    float cy = r.y + r.w * 0.5f;
    dl->AddEllipseFilled(ImVec2(cx, cy), ImVec2(r.z * 0.5f, r.w * 0.5f), glowColor);
}

template<typename FieldDefT>
inline void DrawSvgBindingLabels(const SvgRenderer& svg, std::span<const SvgBindingDef> bindings,
                                  std::span<const FieldDefT> fields, ImVec2 origin, ImVec2 size) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    constexpr float pad = 4.0f;

    for (const auto& b : bindings) {
        ImVec4 r = svg.GetShapeBounds(ShapeIdToString(b.shapeId), origin, size);
        if (r.z <= 0) continue;

        // Line 1: label
        const char* labelText = nullptr;
        if (!b.label.empty()) {
            labelText = b.label.c_str();
        } else {
            labelText = "???";
        }

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
