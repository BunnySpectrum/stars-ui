#pragma once

#include "view_common.h"
#include "../field_defs.h"
#include "../field_store.h"
#include "../field_renderer.h"

#include <span>

// ============================================================================
// Radio view — per-view field enum that maps to unified internal IDs
// ============================================================================

// Per-view field enum for Radio (maps to unified FieldId)
enum class RadioFieldId : int {
    // ACTIVE CHANNELS
    Ch1Subspace,
    Ch2Emergency,
    Ch3Tactical,
    Ch4Science,
    Ch5Command,
    Ch6Diplomatic,

    // SIGNAL STATUS
    SubspaceBandwidth,
    SignalStrength,
    NoiseFloor,
    SNR,
    AntennaArray,
    Range,

    COUNT
};

// ============================================================================
// Radio field definitions table
// ============================================================================

// Radio-specific field definition (uses local RadioFieldId, no ViewId needed)
using RadioFieldDef = FieldDefT<RadioFieldId, FieldId::Ch1Subspace>;

// ============================================================================
// Radio view content — unified fields + graphs (no SVG for this view)
// ============================================================================

namespace radio_detail {

//      ID                                  LABEL                   UNITS   DISPLAY            SECTION                COL   DIR                    WARN    CRIT
inline constexpr RadioFieldDef kFields[] = {
    // ACTIVE CHANNELS
    { RadioFieldId::Ch1Subspace,      "CH 1  SUBSPACE",       "GHz",  Display::Scalar,   "ACTIVE CHANNELS",     0 },
    { RadioFieldId::Ch2Emergency,     "CH 2  EMERGENCY",      "MHz",  Display::Scalar,   "ACTIVE CHANNELS",     0 },
    { RadioFieldId::Ch3Tactical,      "CH 3  TACTICAL",       "GHz",  Display::Scalar,   "ACTIVE CHANNELS",     0 },
    { RadioFieldId::Ch4Science,       "CH 4  SCIENCE",        "GHz",  Display::Scalar,   "ACTIVE CHANNELS",     0 },
    { RadioFieldId::Ch5Command,       "CH 5  COMMAND",        "GHz",  Display::Scalar,   "ACTIVE CHANNELS",     0 },
    { RadioFieldId::Ch6Diplomatic,    "CH 6  DIPLOMATIC",     "GHz",  Display::Scalar,   "ACTIVE CHANNELS",     0 },
    // SIGNAL STATUS
    { RadioFieldId::SubspaceBandwidth,"SUBSPACE BANDWIDTH",   "GHz",  Display::Scalar,   "SIGNAL STATUS",       1 },
    { RadioFieldId::SignalStrength,   "SIGNAL STRENGTH",      "dBm",  Display::Scalar,   "SIGNAL STATUS",       1,    ColorDir::LowIsWorse, -50.0, -60.0 },
    { RadioFieldId::NoiseFloor,       "NOISE FLOOR",          "dBm",  Display::Scalar,   "SIGNAL STATUS",       1 },
    { RadioFieldId::SNR,              "SNR",                  "dB",   Display::Scalar,   "SIGNAL STATUS",       1,    ColorDir::LowIsWorse,  60.0,  50.0 },
    { RadioFieldId::AntennaArray,     "ANTENNA ARRAY",        "",     Display::Scalar,   "SIGNAL STATUS",       1 },
    { RadioFieldId::Range,            "RANGE",                "km",   Display::Scalar,   "SIGNAL STATUS",       1 },
};

//                                      base    xf1   tf1    a1     p1      xf2    tf2    a2     p2
inline constexpr GraphDef kGraphs[] = {
    { GraphId::RadioSignal,  ViewId::Radio,    nullptr,            "FREQUENCY", "dBm",     -120.0f, -20.0f,
      {{ GraphLineId::SignalStrength, kOrange, { -42.0f, 2.0f, 1.5f, 8.0f,  0.0f,   7.3f,  3.1f, 3.0f,  0.0f  }},
       { GraphLineId::Noise,          kPurple, {-110.0f, 3.0f, 0.7f, 2.0f,  0.0f,  11.0f,  2.3f, 1.5f,  1.571f}}} },

    { GraphId::OrbitalTrack, ViewId::Radio,    "ORBITAL TRACK",    "TIME",      "km",   35750.0f, 42000.0f,
      {{ GraphLineId::Altitude,       kOrange, {35786.0f, 0.5f, 0.1f, 10.0f, 0.0f,  0.0f,  0.0f, 0.0f,  0.0f  }},
       { GraphLineId::Range,          kBlue,   {38000.0f, 0.3f, 0.2f, 500.0f, 1.571f, 0.0f,  0.0f, 0.0f,  0.0f  }}} },
};

} // namespace radio_detail

// Unified view content for Radio
inline constexpr ViewContentDef<RadioFieldDef> kRadioContent = {
    .fields = radio_detail::kFields,
    .graphs = radio_detail::kGraphs,
    .svg = nullptr,  // Radio has no SVG
};

// ============================================================================
// Radio view info — exported for the panel to include in its tab list
// ============================================================================

inline const ViewInfo kRadioViewInfo = {
    "RADIO",
    kOrange,
    [](const FieldStore& fs, std::span<const GraphBuffer> gbufs) { DrawViewContent(kRadioContent, fs, gbufs); },
    {},  // no SVG
    { {{{"ALL FREQ", kOrange}, {"SUBSPACE", kOrange}, {"EM BAND", kOrange}}, 3},
      {{{"HAIL", kBlue}, {"ENCRYPT", kRed}}, 2} },
    2,
    2,   // graphCount (2 graphs for RadioSignal and OrbitalTrack)
    UpdateRadioFields,
};
