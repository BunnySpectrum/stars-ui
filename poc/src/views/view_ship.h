#pragma once

#include "view_common.h"
#include "../field_defs.h"
#include "../field_store.h"
#include "../field_renderer.h"
#include "view_tactical.h"
#include "../../assets/ship_wireframe_ids.h"

#include <span>

// ============================================================================
// Ship view — SVG view (ship wireframe with animations and bindings)
// ============================================================================

// Ship-specific struct types using ShipWireframeId instead of unified ShapeId
struct ShipShapeColorDef {
    ShipWireframeId shapeId;
    ImU32           color;
    const char* GetShapeString() const { return ShipWireframeIdToString(shapeId); }
};

struct ShipShapeAnimDef {
    ShipWireframeId shapeId;
    AnimType        type;
    float           frequency;
    ImU32           color1;
    ImU32           color2;
    int             minAlpha;
    const char* GetShapeString() const { return ShipWireframeIdToString(shapeId); }
};

struct ShipGlowOverlayDef {
    ShipWireframeId shapeId;
    float           frequency;
    int             maxAlpha;
    ImU32           color;
    const char* GetShapeString() const { return ShipWireframeIdToString(shapeId); }
};

namespace ship_detail {

using namespace std::string_literals;

// Helper color for detail/grid elements
inline constexpr ImU32 kDetail = IM_COL32(0x66, 0x88, 0xAA, 0xFF);

// Default shape colors
inline constexpr ShipShapeColorDef kDefaultColors[] = {
    {ShipWireframeId::Shield,          kBlue},
    {ShipWireframeId::Saucer,          kBlue},
    {ShipWireframeId::Bridge,          kOrange},
    {ShipWireframeId::SaucerGrid,      kDetail},
    {ShipWireframeId::Neck,            kBlue},
    {ShipWireframeId::EngineeringHull, kBlue},
    {ShipWireframeId::Deflector,       kOrange},
    {ShipWireframeId::PylonLeft,       kBlue},
    {ShipWireframeId::PylonRight,      kBlue},
    {ShipWireframeId::NacelleLeft,     kPurple},
    {ShipWireframeId::NacelleRight,    kPurple},
    {ShipWireframeId::BussardLeft,     kRed},
    {ShipWireframeId::BussardRight,    kRed},
    {ShipWireframeId::NacelleGlowLeft, kDetail},
    {ShipWireframeId::NacelleGlowRight,kDetail},
};

// Shape animations (nacelle blink)
inline constexpr ShipShapeAnimDef kAnimations[] = {
    {ShipWireframeId::NacelleLeft,  AnimType::Blink, 1.0f, kPurple, kRed, 0},
    {ShipWireframeId::NacelleRight, AnimType::Blink, 1.0f, kPurple, kRed, 0},
};

// Glow overlay (saucer pulse)
inline constexpr ShipGlowOverlayDef kGlowOverlay = {
    ShipWireframeId::Saucer, 4.0f, 40, IM_COL32(0x99, 0x99, 0xFF, 0)
};

// SVG bindings — connect fields to SVG shapes with labels and thresholds
inline const SvgBindingDef kSvgBindings[] = {
    //  FIELD                        SHAPE                          ANCHOR               LABEL         FMT       NORMAL   DIR                   WARN   CRIT
    {FieldId::TacShields,       ShapeId::Ship_Shield,          LabelAnchor::Right,  "SHIELDS"s,   "%.0f%%",   kBlue,   ColorDir::LowIsWorse, 50.0, 25.0},
    {FieldId::TacHullIntegrity, ShapeId::Ship_EngineeringHull, LabelAnchor::Center, "HULL"s,      "%.1f%%",   kBlue,   ColorDir::LowIsWorse, 60.0, 30.0},
    {FieldId::TacWarpCore,      ShapeId::Ship_Deflector,       LabelAnchor::Below,  "WARP CORE"s, "%.1f%%",   kOrange, ColorDir::LowIsWorse, 50.0, 25.0},
    {FieldId::TacLifeSupport,   ShapeId::Ship_Bridge,          LabelAnchor::Above,  "LIFE SUPT"s, "%.0f%%",   kOrange, ColorDir::LowIsWorse, 50.0, 25.0},
};

} // namespace ship_detail

// ============================================================================
// Ship SVG content definition
// ============================================================================

class SvgRenderer;

struct ShipSvgContent {
    const char* svgPath = "poc/assets/ship_wireframe.svg";
    ImU32       strokeColor = kBlue;
    float       strokeWidth = 1.5f;

    std::span<const ShipShapeColorDef> defaultColors = ship_detail::kDefaultColors;
    std::span<const ShipShapeAnimDef>  animations = ship_detail::kAnimations;
    std::span<const SvgBindingDef>     svgBindings = ship_detail::kSvgBindings;
    ShipGlowOverlayDef                 glowOverlay = ship_detail::kGlowOverlay;

    // Field definitions for label lookup (Ship binds to Tactical fields)
    std::span<const TacticalFieldDef>  bindingFields = tactical_detail::kFields;

    // Runtime state (set by TacticalPanel during init)
    SvgRenderer* svg = nullptr;
};

// Non-const so svg pointer can be set at runtime
inline ShipSvgContent kShipSvgContent;

// ============================================================================
// Ship view info — exported for the panel to include in its tab list
// ============================================================================

inline const ViewInfo kShipViewInfo = {
    "SHIP",
    kTan,
    [](const FieldStore& fs, std::span<const GraphBuffer>) { DrawViewContent(kShipSvgContent, fs, {}); },
    [](SvgRenderer& svg) {
        svg.LoadFromFile(kShipSvgContent.svgPath);
        ApplyDefaultColors(svg, kShipSvgContent.defaultColors);
        kShipSvgContent.svg = &svg;
    },
    {},
    0,
    0,   // graphCount (no graphs)
    UpdateShipFields,
};
