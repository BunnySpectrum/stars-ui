#pragma once

#include "view_common.h"
#include "../field_defs.h"
#include "../field_renderer.h"

#include <span>

// ============================================================================
// Tactical view — per-view field enum that maps to unified internal IDs
// ============================================================================

// Per-view field enum for Tactical (maps to unified FieldId)
enum class TacticalFieldId : int {
    WarpCore,
    Shields,
    HullIntegrity,
    LifeSupport,

    COUNT
};

// ============================================================================
// Tactical field definitions table
// ============================================================================

// Tactical-specific field definition (uses local TacticalFieldId, no ViewId needed)
using TacticalFieldDef = FieldDefT<TacticalFieldId, FieldId::TacWarpCore>;

// ============================================================================
// Tactical view content — fields + graphs
// ============================================================================

namespace tactical_detail {

//      ID                              LABEL              UNITS   DISPLAY          SECTION           COL
inline constexpr TacticalFieldDef kFields[] = {
    { TacticalFieldId::WarpCore,      "WARP CORE",       "",     Display::Scalar, "SYSTEM STATUS",  0 },
    { TacticalFieldId::Shields,       "SHIELDS",         "",     Display::Scalar, "SYSTEM STATUS",  0 },
    { TacticalFieldId::HullIntegrity, "HULL INTEGRITY",  "",     Display::Scalar, "SYSTEM STATUS",  0 },
    { TacticalFieldId::LifeSupport,   "LIFE SUPPORT",    "",     Display::Scalar, "SYSTEM STATUS",  0 },
};

//                                      base    xf1   tf1    a1     p1      xf2    tf2    a2     p2
inline constexpr GraphDef kGraphs[] = {
    { GraphId::Telemetry, ViewId::Tactical, "MAG FIELD (nT)", "TIME", "nT", -1.5f, 1.5f,
      {{ GraphLineId::MagFieldBx, kOrange, { 0.0f, 1.0f, 1.0f, 0.5f, 0.0f, 2.3f, 1.7f, 0.5f, 0.0f }},
       { GraphLineId::MagFieldBy, kBlue,   { 0.0f, 0.7f, 0.8f, 0.8f, 1.571f, 0.0f, 0.0f, 0.0f, 0.0f }}} },
};

} // namespace tactical_detail

// Unified view content for Tactical
inline constexpr ViewContentDef<TacticalFieldDef> kTacticalContent = {
    .fields = tactical_detail::kFields,
    .graphs = tactical_detail::kGraphs,
    .svg = nullptr,
};

// ============================================================================
// Tactical view info — exported for the panel to include in its tab list
// ============================================================================

inline const ViewInfo kTacticalViewInfo = {
    "TACTICAL",
    kPurple,
    [] { DrawViewContent(kTacticalContent); },
    {},  // no SVG
    {},
    0
};
