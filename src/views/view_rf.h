#pragma once

#include "view_common.h"
#include "../field_defs.h"
#include "../field_renderer.h"
#include "../../assets/rf_circuit_ids.h"

#include <span>

// ============================================================================
// RF view — SVG-only view (RF circuit schematic with animations and bindings)
// ============================================================================

// RF-specific struct types using RfCircuitId instead of unified ShapeId
struct RfShapeColorDef {
    RfCircuitId shapeId;
    ImU32       color;
    const char* GetShapeString() const { return RfCircuitIdToString(shapeId); }
};

struct RfShapeAnimDef {
    RfCircuitId shapeId;
    AnimType    type;
    float       frequency;
    ImU32       color1;
    ImU32       color2;
    int         minAlpha;
    const char* GetShapeString() const { return RfCircuitIdToString(shapeId); }
};

namespace rf_detail {

// Default shape colors
inline constexpr RfShapeColorDef kDefaultColors[] = {
    {RfCircuitId::OscillatorY1, kOrange},
    {RfCircuitId::OscillatorY2, kOrange},
    {RfCircuitId::MixerU2,      kBlue},
    {RfCircuitId::MixerU3,      kBlue},
    {RfCircuitId::OpampU4,      kPurple},
    {RfCircuitId::DacU1,        kTan},
    {RfCircuitId::AntennaAe1,   kOrange},
};

// Shape animations
inline constexpr RfShapeAnimDef kAnimations[] = {
    {RfCircuitId::MixerU2,      AnimType::AlphaPulse, 4.0f, IM_COL32(0x99, 0x99, 0xFF, 0), 0, 128},
    {RfCircuitId::MixerU3,      AnimType::AlphaPulse, 4.0f, IM_COL32(0x99, 0x99, 0xFF, 0), 0, 128},
    {RfCircuitId::OscillatorY1, AnimType::AlphaPulse, 6.0f, IM_COL32(0xFF, 0x99, 0x33, 0), 0, 128},
    {RfCircuitId::OscillatorY2, AnimType::AlphaPulse, 6.0f, IM_COL32(0xFF, 0x99, 0x33, 0), 0, 128},
};

// SVG bindings — connect fields to SVG shapes with labels and thresholds
inline constexpr SvgBindingDef kSvgBindings[] = {
    //  FIELD                   VIEW        SHAPE               ANCHOR              LABEL     FMT         NORMAL   DIR                  WARN    CRIT
    {FieldId::SignalStrength, ViewId::Rf, ShapeId::Rf_MixerU2, LabelAnchor::Right, "SIGNAL", "%.0f dBm", kOrange, ColorDir::LowIsWorse, -50.0, -60.0},
};

} // namespace rf_detail

// ============================================================================
// RF SVG content definition
// ============================================================================

class SvgRenderer;

struct RfSvgContent {
    const char* svgPath = "assets/rf_circuit.svg";
    ImU32       strokeColor = kBlue;
    float       strokeWidth = 1.5f;

    std::span<const RfShapeColorDef> defaultColors = rf_detail::kDefaultColors;
    std::span<const RfShapeAnimDef>  animations = rf_detail::kAnimations;
    std::span<const SvgBindingDef>   svgBindings = rf_detail::kSvgBindings;

    // Runtime state (set by TacticalPanel during init)
    SvgRenderer* svg = nullptr;
};

// Non-const so svg pointer can be set at runtime
inline RfSvgContent kRfSvgContent;

// ============================================================================
// RF view info — exported for the panel to include in its tab list
// ============================================================================

inline const ViewInfo kRfViewInfo = {
    "RF",
    kTan,
    [] { DrawViewContent(kRfSvgContent); },
    [](SvgRenderer& svg) {
        svg.LoadFromFile(kRfSvgContent.svgPath);
        ApplyDefaultColors(svg, kRfSvgContent.defaultColors);
        kRfSvgContent.svg = &svg;
    },
    {},
    0
};
