#pragma once

#include "view_common.h"
#include "../field_defs.h"
#include "../field_store.h"
#include "../field_renderer.h"

#include <span>

// ============================================================================
// Power view — per-view field enum that maps to unified internal IDs
// ============================================================================

// Per-view field enum for Power (maps to unified FieldId)
enum class PowerFieldId : int {
    // POWER GENERATION
    GenWarpCore,
    GenImpulse,
    GenSolar,
    GenAuxFusion,
    GenTotal,

    // POWER CONSUMPTION
    ConPropulsion,
    ConShields,
    ConLifeSupport,
    ConSensors,
    ConComputers,
    ConWeapons,
    ConTotal,

    // ENERGY RESERVES
    BatteryA,
    BatteryB,
    Emergency,
    Surplus,

    COUNT
};

// ============================================================================
// Power field definitions table
// ============================================================================

// Power-specific field definition (uses local PowerFieldId, no ViewId needed)
using PowerFieldDef = FieldDefT<PowerFieldId, FieldId::GenWarpCore>;

// ============================================================================
// Power view content — fields + graphs
// ============================================================================

namespace power_detail {

//      ID                            LABEL            UNITS   DISPLAY          SECTION              COL
inline constexpr PowerFieldDef kFields[] = {
    // POWER GENERATION
    { PowerFieldId::GenWarpCore,    "WARP CORE",     "MW",   Display::Scalar, "POWER GENERATION",  0 },
    { PowerFieldId::GenImpulse,     "IMPULSE",       "MW",   Display::Scalar, "POWER GENERATION",  0 },
    { PowerFieldId::GenSolar,       "SOLAR",         "MW",   Display::Scalar, "POWER GENERATION",  0 },
    { PowerFieldId::GenAuxFusion,   "AUX FUSION",    "MW",   Display::Scalar, "POWER GENERATION",  0 },
    { PowerFieldId::GenTotal,       "TOTAL",         "MW",   Display::Scalar, "POWER GENERATION",  0 },
    // POWER CONSUMPTION
    { PowerFieldId::ConPropulsion,  "PROPULSION",    "MW",   Display::Scalar, "POWER CONSUMPTION", 1 },
    { PowerFieldId::ConShields,     "SHIELDS",       "MW",   Display::Scalar, "POWER CONSUMPTION", 1 },
    { PowerFieldId::ConLifeSupport, "LIFE SUPPORT",  "MW",   Display::Scalar, "POWER CONSUMPTION", 1 },
    { PowerFieldId::ConSensors,     "SENSORS",       "MW",   Display::Scalar, "POWER CONSUMPTION", 1 },
    { PowerFieldId::ConComputers,   "COMPUTERS",     "MW",   Display::Scalar, "POWER CONSUMPTION", 1 },
    { PowerFieldId::ConWeapons,     "WEAPONS",       "MW",   Display::Scalar, "POWER CONSUMPTION", 1 },
    { PowerFieldId::ConTotal,       "TOTAL",         "MW",   Display::Scalar, "POWER CONSUMPTION", 1 },
    // ENERGY RESERVES
    { PowerFieldId::BatteryA,       "BATTERY A",     "%",    Display::Scalar, "ENERGY RESERVES",   2 },
    { PowerFieldId::BatteryB,       "BATTERY B",     "%",    Display::Scalar, "ENERGY RESERVES",   2 },
    { PowerFieldId::Emergency,      "EMERGENCY",     "%",    Display::Scalar, "ENERGY RESERVES",   2 },
    { PowerFieldId::Surplus,        "SURPLUS",       "MW",   Display::Scalar, "ENERGY RESERVES",   2 },
};

//                                      base    xf1   tf1    a1     p1      xf2    tf2    a2     p2
inline constexpr GraphDef kGraphs[] = {
    { GraphId::PowerTrend, ViewId::Power, nullptr, "TIME", "MW", 1100.0f, 1800.0f,
      {{ GraphLineId::GridLoad,   kOrange, { 1300.0f, 0.8f, 0.4f, 50.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
       { GraphLineId::Generation, kBlue,   { 1696.0f, 0.6f, 0.3f, 30.0f, 1.571f, 0.0f, 0.0f, 0.0f, 0.0f }}} },
};

} // namespace power_detail

// Unified view content for Power
inline constexpr ViewContentDef<PowerFieldDef> kPowerContent = {
    .fields = power_detail::kFields,
    .graphs = power_detail::kGraphs,
    .svg = nullptr,
};

// ============================================================================
// Power view info — exported for the panel to include in its tab list
// ============================================================================

inline const ViewInfo kPowerViewInfo = {
    "POWER",
    kTan,
    [](const FieldStore& fs, std::span<const GraphBuffer> gbufs) { DrawViewContent(kPowerContent, fs, gbufs); },
    {},  // no SVG
    { {{{"OVERVIEW", kTan}, {"GRID MAP", kTan}}, 2},
      {{{"DIVERT", kRed}, {"BALANCE", kBlue}}, 2} },
    2,
    1,   // graphCount (1 graph for PowerTrend)
    UpdatePowerFields,
};
