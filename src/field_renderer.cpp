#include "field_renderer.h"
#include "svg_renderer.h"

#include <cmath>

// Template implementations are in field_renderer.h

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

// ============================================================================
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

