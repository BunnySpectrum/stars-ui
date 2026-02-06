#pragma once

#include "view_common.h"
#include "../field_defs.h"
#include "../field_renderer.h"

#include <span>

// ============================================================================
// Info view — per-view field enum that maps to unified internal IDs
// ============================================================================

// Per-view field enum for Info (maps to unified FieldId)
enum class InfoFieldId : int {
    // MISSION TIMERS
    MissionElapsed,
    Uptime,
    TimeToEclipse,
    NextMaintenance,

    // CHRONOMETER
    PacificTime,
    Utc,
    Stardate,

    COUNT
};

// ============================================================================
// Info field definitions table
// ============================================================================

// Info-specific field definition (uses local InfoFieldId, no ViewId needed)
using InfoFieldDef = FieldDefT<InfoFieldId, FieldId::MissionElapsed>;

// ============================================================================
// Info view content — fields only (no graphs or SVG for this view)
// ============================================================================

namespace info_detail {

//      ID                              LABEL                UNITS   DISPLAY             SECTION              COL
inline constexpr InfoFieldDef kFields[] = {
    // MISSION TIMERS
    { InfoFieldId::MissionElapsed,    "MISSION ELAPSED",   "",     Display::TimerUp,   "MISSION TIMERS",    0 },
    { InfoFieldId::Uptime,            "UPTIME",            "",     Display::TimerUp,   "MISSION TIMERS",    0 },
    { InfoFieldId::TimeToEclipse,     "TIME TO ECLIPSE",   "",     Display::TimerDown, "MISSION TIMERS",    0 },
    { InfoFieldId::NextMaintenance,   "NEXT MAINTENANCE",  "",     Display::TimerDown, "MISSION TIMERS",    0 },
    // CHRONOMETER
    { InfoFieldId::PacificTime,       "PACIFIC TIME",      "",     Display::Clock,     "CHRONOMETER",       1 },
    { InfoFieldId::Utc,               "UTC",               "",     Display::Clock,     "CHRONOMETER",       1 },
    { InfoFieldId::Stardate,          "STARDATE",          "",     Display::Scalar,    "CHRONOMETER",       1 },
};

} // namespace info_detail

// Unified view content for Info
inline constexpr ViewContentDef<InfoFieldDef> kInfoContent = {
    .fields = info_detail::kFields,
    .graphs = {},       // Info has no graphs
    .svg = nullptr,     // Info has no SVG
};

// ============================================================================
// Info view info — exported for the panel to include in its view list
// ============================================================================

inline const ViewInfo kInfoViewInfo = {
    "INFO",
    kBlue,
    [] { DrawViewContent(kInfoContent); },
    {},  // no SVG
    {},  // no option groups
    0
};
