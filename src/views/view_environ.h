#pragma once

#include "view_common.h"
#include "../field_defs.h"
#include "../field_renderer.h"

#include <span>

// ============================================================================
// Environ view — per-view field enum that maps to unified internal IDs
// ============================================================================

// Per-view field enum for Environ (maps to unified FieldId)
enum class EnvironFieldId : int {
    // ATMOSPHERIC CONDITIONS
    Temperature,
    Humidity,
    Pressure,
    AirQuality,
    CO2,
    O2,

    // DECK STATUS
    Deck1Bridge,
    Deck2Officers,
    Deck3Crew,
    Deck5Science,
    Deck7Engineering,
    Deck10Cargo,

    COUNT
};

// ============================================================================
// Environ field definitions table
// ============================================================================

// Environ-specific field definition (uses local EnvironFieldId, no ViewId needed)
struct EnvironFieldDef {
    EnvironFieldId id;
    const char*    label;
    const char*    units;
    Display        display;
    const char*    section;
    int            column;

    // Convert to unified FieldId for FieldStore access
    FieldId GetFieldId() const {
        return static_cast<FieldId>(
            static_cast<int>(FieldId::EnvTemperature) + static_cast<int>(id)
        );
    }
};

// ============================================================================
// Environ view content — fields + graphs
// ============================================================================

namespace environ_detail {

//      ID                               LABEL                 UNITS   DISPLAY          SECTION                    COL
inline constexpr EnvironFieldDef kFields[] = {
    // ATMOSPHERIC CONDITIONS
    { EnvironFieldId::Temperature,     "TEMPERATURE",        "C",    Display::Scalar, "ATMOSPHERIC CONDITIONS",  0 },
    { EnvironFieldId::Humidity,        "HUMIDITY",           "%",    Display::Scalar, "ATMOSPHERIC CONDITIONS",  0 },
    { EnvironFieldId::Pressure,        "PRESSURE",           "kPa",  Display::Scalar, "ATMOSPHERIC CONDITIONS",  0 },
    { EnvironFieldId::AirQuality,      "AIR QUALITY",        "",     Display::Scalar, "ATMOSPHERIC CONDITIONS",  0 },
    { EnvironFieldId::CO2,             "CO2",                "PPM",  Display::Scalar, "ATMOSPHERIC CONDITIONS",  0 },
    { EnvironFieldId::O2,              "O2",                 "%",    Display::Scalar, "ATMOSPHERIC CONDITIONS",  0 },
    // DECK STATUS
    { EnvironFieldId::Deck1Bridge,     "DECK 1 BRIDGE",      "C",    Display::Scalar, "DECK STATUS",             1 },
    { EnvironFieldId::Deck2Officers,   "DECK 2 OFFICERS",    "C",    Display::Scalar, "DECK STATUS",             1 },
    { EnvironFieldId::Deck3Crew,       "DECK 3 CREW",        "C",    Display::Scalar, "DECK STATUS",             1 },
    { EnvironFieldId::Deck5Science,    "DECK 5 SCIENCE",     "C",    Display::Scalar, "DECK STATUS",             1 },
    { EnvironFieldId::Deck7Engineering,"DECK 7 ENGINEERING", "C",    Display::Scalar, "DECK STATUS",             1 },
    { EnvironFieldId::Deck10Cargo,     "DECK 10 CARGO",      "C",    Display::Scalar, "DECK STATUS",             1 },
};

//                                      base    xf1   tf1    a1     p1      xf2    tf2    a2     p2
inline constexpr GraphDef kGraphs[] = {
    { GraphId::EnvironTrend, ViewId::Environ, nullptr, "TIME", "VALUE", 18.0f, 55.0f,
      {{ GraphLineId::Temperature, kOrange, { 22.0f, 0.5f, 0.3f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
       { GraphLineId::Humidity,    kBlue,   { 47.0f, 0.3f, 0.2f, 2.0f, 1.571f, 0.0f, 0.0f, 0.0f, 0.0f }}} },
};

} // namespace environ_detail

// Unified view content for Environ
inline constexpr ViewContentDef<EnvironFieldDef> kEnvironContent = {
    .fields = environ_detail::kFields,
    .graphs = environ_detail::kGraphs,
    .svg = nullptr,
};

// ============================================================================
// Environ view info — exported for the panel to include in its tab list
// ============================================================================

inline const ViewInfo kEnvironViewInfo = {
    "ENVIRON",
    kBlue,
    [] { DrawViewContent(kEnvironContent); },
    {},  // no SVG
    { {{{"ALL DECKS", kBlue}, {"BRIDGE", kBlue}, {"ENGINEERING", kBlue}}, 3},
      {{{"ALERTS", kRed}}, 1} },
    2
};
