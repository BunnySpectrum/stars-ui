#pragma once

#include "view_common.h"
#include "../field_defs.h"
#include "../field_renderer.h"
#include "../../assets/goes_orbit_ids.h"

#include <span>

// ============================================================================
// Map view — SVG view (GOES orbital map with animations and bindings)
// ============================================================================

// Map-specific struct types using GoesOrbitId instead of unified ShapeId
struct MapShapeColorDef {
    GoesOrbitId shapeId;
    ImU32       color;
    const char* GetShapeString() const { return GoesOrbitIdToString(shapeId); }
};

struct MapShapeAnimDef {
    GoesOrbitId shapeId;
    AnimType    type;
    float       frequency;
    ImU32       color1;
    ImU32       color2;
    int         minAlpha;
    const char* GetShapeString() const { return GoesOrbitIdToString(shapeId); }
};

namespace map_detail {

// Helper color for detail/grid elements
inline constexpr ImU32 kDetail = IM_COL32(0x66, 0x88, 0xAA, 0xFF);

// Default shape colors
inline constexpr MapShapeColorDef kDefaultColors[] = {
    {GoesOrbitId::GeoOrbit,          kDetail},
    {GoesOrbitId::Earth,             kBlue},
    {GoesOrbitId::EarthGrid,         kDetail},
    {GoesOrbitId::Equator,           kBlue},
    {GoesOrbitId::GroundStation,     kOrange},
    {GoesOrbitId::Goes16,            kBlue},
    {GoesOrbitId::LinkLine,          kOrange},
    {GoesOrbitId::SignalCone,        kDetail},
    {GoesOrbitId::SubsatellitePoint, kPurple},
    {GoesOrbitId::CoverageArc,       kPurple},
    {GoesOrbitId::LonGrid,           kDetail},
};

// Shape animations (satellite pulse)
inline constexpr MapShapeAnimDef kAnimations[] = {
    {GoesOrbitId::Goes16, AnimType::AlphaPulse, 3.0f, IM_COL32(0x99, 0x99, 0xFF, 0), 0, 128},
};

// SVG bindings — connect fields to SVG shapes with labels and thresholds
inline constexpr SvgBindingDef kSvgBindings[] = {
    //  FIELD                   VIEW        SHAPE                ANCHOR              LABEL     FMT         NORMAL   DIR                   WARN    CRIT
    {FieldId::SignalStrength, ViewId::Map, ShapeId::Map_LinkLine, LabelAnchor::Left,  "SIGNAL", "%.0f dBm", kOrange, ColorDir::LowIsWorse, -50.0, -60.0},
    {FieldId::SNR,            ViewId::Map, ShapeId::Map_Goes16,   LabelAnchor::Above, "SNR",    "%.0f dB",  kBlue,   ColorDir::LowIsWorse,  60.0,  50.0},
};

} // namespace map_detail

// ============================================================================
// Map SVG content definition
// ============================================================================

class SvgRenderer;

struct MapSvgContent {
    const char* svgPath = "assets/goes_orbit.svg";
    ImU32       strokeColor = kBlue;
    float       strokeWidth = 1.5f;

    std::span<const MapShapeColorDef> defaultColors = map_detail::kDefaultColors;
    std::span<const MapShapeAnimDef>  animations = map_detail::kAnimations;
    std::span<const SvgBindingDef>    svgBindings = map_detail::kSvgBindings;

    // Runtime state (set by TacticalPanel during init)
    SvgRenderer* svg = nullptr;
};

// Non-const so svg pointer can be set at runtime
inline MapSvgContent kMapSvgContent;

// ============================================================================
// Map view info — exported for the panel to include in its tab list
// ============================================================================

inline const ViewInfo kMapViewInfo = {
    "MAP",
    kBlue,
    [] { DrawViewContent(kMapSvgContent); },
    [](SvgRenderer& svg) {
        svg.LoadFromFile(kMapSvgContent.svgPath);
        ApplyDefaultColors(svg, kMapSvgContent.defaultColors);
        kMapSvgContent.svg = &svg;
    },
    {},
    0
};
