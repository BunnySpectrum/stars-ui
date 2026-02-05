#pragma once

#include "stars.h"
#include "views/view_common.h"

#include "../assets/ship_wireframe_ids.h"
#include "../assets/goes_orbit_ids.h"
#include "../assets/rf_circuit_ids.h"

// ============================================================================
// View IDs — each view has its own header in views/view_*.h
// ============================================================================

enum class ViewId : int {
    Tactical,
    Ship,
    Environ,
    Power,
    Radio,
    Map,
    Rf,

    // INFO panel is separate (not in TacticalPanel)
    Info,

    COUNT
};

// ============================================================================
// Field IDs — add entries here, then add a corresponding row to kFields[].
// ============================================================================

enum class FieldId : int {
    // INFO — MISSION TIMERS (column 0)
    MissionElapsed,
    Uptime,
    TimeToEclipse,
    NextMaintenance,

    // INFO — CHRONOMETER (column 1)
    PacificTime,
    Utc,
    Stardate,

    // TACTICAL — SYSTEM STATUS
    TacWarpCore,
    TacShields,
    TacHullIntegrity,
    TacLifeSupport,

    // ENVIRON — ATMOSPHERIC CONDITIONS
    EnvTemperature,
    EnvHumidity,
    EnvPressure,
    EnvAirQuality,
    EnvCO2,
    EnvO2,

    // ENVIRON — DECK STATUS
    Deck1Bridge,
    Deck2Officers,
    Deck3Crew,
    Deck5Science,
    Deck7Engineering,
    Deck10Cargo,

    // POWER — POWER GENERATION
    GenWarpCore,
    GenImpulse,
    GenSolar,
    GenAuxFusion,
    GenTotal,

    // POWER — POWER CONSUMPTION
    ConPropulsion,
    ConShields,
    ConLifeSupport,
    ConSensors,
    ConComputers,
    ConWeapons,
    ConTotal,

    // POWER — ENERGY RESERVES
    BatteryA,
    BatteryB,
    Emergency,
    Surplus,

    // RADIO — ACTIVE CHANNELS
    Ch1Subspace,
    Ch2Emergency,
    Ch3Tactical,
    Ch4Science,
    Ch5Command,
    Ch6Diplomatic,

    // RADIO — SIGNAL STATUS
    SubspaceBandwidth,
    SignalStrength,
    NoiseFloor,
    SNR,
    AntennaArray,
    Range,

    COUNT
};

// ============================================================================
// Field definition table — edit this to change what fields appear and where.
//
//   id        — unique FieldId (must match enum above)
//   label     — display label shown in the left column
//   units     — informational unit string (e.g. "C", "MW", "%")
//   display   — how the update function formats this field's value
//   view      — which panel view this field belongs to
//   section   — section header text (fields with the same section are grouped)
//   column    — 0 = left/default, 1 = right (for side-by-side layout)
// ============================================================================

struct FieldDef {
    FieldId     id;
    const char* label;
    const char* units;
    Display     display;
    ViewId      view;
    const char* section;
    int         column;

    // For compatibility with per-view field defs (template drawing functions)
    FieldId GetFieldId() const { return id; }
};

//      ID                      LABEL                   UNITS   DISPLAY            VIEW             SECTION                       COL
inline const FieldDef kFields[] = {
    // INFO panel — left side
    { FieldId::MissionElapsed,  "MISSION ELAPSED",      "",     Display::TimerUp,   ViewId::Info,    "MISSION TIMERS",             0 },
    { FieldId::Uptime,          "UPTIME",               "",     Display::TimerUp,   ViewId::Info,    "MISSION TIMERS",             0 },
    { FieldId::TimeToEclipse,   "TIME TO ECLIPSE",      "",     Display::TimerDown, ViewId::Info,    "MISSION TIMERS",             0 },
    { FieldId::NextMaintenance, "NEXT MAINTENANCE",     "",     Display::TimerDown, ViewId::Info,    "MISSION TIMERS",             0 },
    // INFO panel — right side
    { FieldId::PacificTime,     "PACIFIC TIME",         "",     Display::Clock,     ViewId::Info,    "CHRONOMETER",                1 },
    { FieldId::Utc,             "UTC",                  "",     Display::Clock,     ViewId::Info,    "CHRONOMETER",                1 },
    { FieldId::Stardate,        "STARDATE",             "",     Display::Scalar,    ViewId::Info,    "CHRONOMETER",                1 },

    // TACTICAL panel
    { FieldId::TacWarpCore,     "WARP CORE",            "",     Display::Scalar,    ViewId::Tactical,"SYSTEM STATUS",              0 },
    { FieldId::TacShields,      "SHIELDS",              "",     Display::Scalar,    ViewId::Tactical,"SYSTEM STATUS",              0 },
    { FieldId::TacHullIntegrity,"HULL INTEGRITY",       "",     Display::Scalar,    ViewId::Tactical,"SYSTEM STATUS",              0 },
    { FieldId::TacLifeSupport,  "LIFE SUPPORT",         "",     Display::Scalar,    ViewId::Tactical,"SYSTEM STATUS",              0 },

    // ENVIRON — atmospheric
    { FieldId::EnvTemperature,  "TEMPERATURE",          "C",    Display::Scalar,    ViewId::Environ, "ATMOSPHERIC CONDITIONS",     0 },
    { FieldId::EnvHumidity,     "HUMIDITY",             "%",    Display::Scalar,    ViewId::Environ, "ATMOSPHERIC CONDITIONS",     0 },
    { FieldId::EnvPressure,     "PRESSURE",             "kPa",  Display::Scalar,    ViewId::Environ, "ATMOSPHERIC CONDITIONS",     0 },
    { FieldId::EnvAirQuality,   "AIR QUALITY",          "",     Display::Scalar,    ViewId::Environ, "ATMOSPHERIC CONDITIONS",     0 },
    { FieldId::EnvCO2,          "CO2",                  "PPM",  Display::Scalar,    ViewId::Environ, "ATMOSPHERIC CONDITIONS",     0 },
    { FieldId::EnvO2,           "O2",                   "%",    Display::Scalar,    ViewId::Environ, "ATMOSPHERIC CONDITIONS",     0 },
    // ENVIRON — decks
    { FieldId::Deck1Bridge,     "DECK 1 BRIDGE",        "C",    Display::Scalar,    ViewId::Environ, "DECK STATUS",                1 },
    { FieldId::Deck2Officers,   "DECK 2 OFFICERS",      "C",    Display::Scalar,    ViewId::Environ, "DECK STATUS",                1 },
    { FieldId::Deck3Crew,       "DECK 3 CREW",          "C",    Display::Scalar,    ViewId::Environ, "DECK STATUS",                1 },
    { FieldId::Deck5Science,    "DECK 5 SCIENCE",       "C",    Display::Scalar,    ViewId::Environ, "DECK STATUS",                1 },
    { FieldId::Deck7Engineering,"DECK 7 ENGINEERING",   "C",    Display::Scalar,    ViewId::Environ, "DECK STATUS",                1 },
    { FieldId::Deck10Cargo,     "DECK 10 CARGO",        "C",    Display::Scalar,    ViewId::Environ, "DECK STATUS",                1 },

    // POWER — generation
    { FieldId::GenWarpCore,     "WARP CORE",            "MW",   Display::Scalar,    ViewId::Power,   "POWER GENERATION",           0 },
    { FieldId::GenImpulse,      "IMPULSE",              "MW",   Display::Scalar,    ViewId::Power,   "POWER GENERATION",           0 },
    { FieldId::GenSolar,        "SOLAR",                "MW",   Display::Scalar,    ViewId::Power,   "POWER GENERATION",           0 },
    { FieldId::GenAuxFusion,    "AUX FUSION",           "MW",   Display::Scalar,    ViewId::Power,   "POWER GENERATION",           0 },
    { FieldId::GenTotal,        "TOTAL",                "MW",   Display::Scalar,    ViewId::Power,   "POWER GENERATION",           0 },
    // POWER — consumption
    { FieldId::ConPropulsion,   "PROPULSION",           "MW",   Display::Scalar,    ViewId::Power,   "POWER CONSUMPTION",          1 },
    { FieldId::ConShields,      "SHIELDS",              "MW",   Display::Scalar,    ViewId::Power,   "POWER CONSUMPTION",          1 },
    { FieldId::ConLifeSupport,  "LIFE SUPPORT",         "MW",   Display::Scalar,    ViewId::Power,   "POWER CONSUMPTION",          1 },
    { FieldId::ConSensors,      "SENSORS",              "MW",   Display::Scalar,    ViewId::Power,   "POWER CONSUMPTION",          1 },
    { FieldId::ConComputers,    "COMPUTERS",            "MW",   Display::Scalar,    ViewId::Power,   "POWER CONSUMPTION",          1 },
    { FieldId::ConWeapons,      "WEAPONS",              "MW",   Display::Scalar,    ViewId::Power,   "POWER CONSUMPTION",          1 },
    { FieldId::ConTotal,        "TOTAL",                "MW",   Display::Scalar,    ViewId::Power,   "POWER CONSUMPTION",          1 },
    // POWER — reserves
    { FieldId::BatteryA,        "BATTERY A",            "%",    Display::Scalar,    ViewId::Power,   "ENERGY RESERVES",            2 },
    { FieldId::BatteryB,        "BATTERY B",            "%",    Display::Scalar,    ViewId::Power,   "ENERGY RESERVES",            2 },
    { FieldId::Emergency,       "EMERGENCY",            "%",    Display::Scalar,    ViewId::Power,   "ENERGY RESERVES",            2 },
    { FieldId::Surplus,         "SURPLUS",              "MW",   Display::Scalar,    ViewId::Power,   "ENERGY RESERVES",            2 },

    // RADIO fields are now defined in views/view_radio.h
};

inline constexpr int kNumFields = sizeof(kFields) / sizeof(kFields[0]);

// ============================================================================
// Graph IDs — add entries here, then add a corresponding row to kGraphs[].
// ============================================================================

enum class GraphId : int {
    Telemetry,      // TACTICAL - magnetic field
    EnvironTrend,   // ENVIRON - temperature/humidity
    PowerTrend,     // POWER - grid load/generation
    RadioSignal,    // RADIO - signal/noise
    OrbitalTrack,   // RADIO - altitude/range

    COUNT
};

// ImPlot ID strings (prefixed with ## for hidden IDs)
inline const char* const kGraphIdStrings[] = {
    "##Telemetry",
    "##EnvironTrend",
    "##PowerTrend",
    "##RadioSignal",
    "##OrbitalTrack",
};

inline const char* GraphIdToPlotId(GraphId id) {
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= static_cast<int>(GraphId::COUNT)) return "##Unknown";
    return kGraphIdStrings[idx];
}

// ============================================================================
// Graph line IDs — identifies each data series for graphs
// ============================================================================

enum class GraphLineId : int {
    // Telemetry (TACTICAL)
    MagFieldBx,
    MagFieldBy,
    // EnvironTrend (ENVIRON)
    Temperature,
    Humidity,
    // PowerTrend (POWER)
    GridLoad,
    Generation,
    // RadioSignal (RADIO)
    SignalStrength,
    Noise,
    // OrbitalTrack (RADIO)
    Altitude,
    Range,

    COUNT,
    None = -1  // For unused line slots
};

// Display labels for graph lines (used in ImPlot legend)
inline const char* const kGraphLineLabels[] = {
    "Bx",
    "By",
    "Temperature (C)",
    "Humidity (%)",
    "Grid Load",
    "Generation",
    "Signal Strength",
    "Noise",
    "Altitude",
    "Range",
};

inline const char* GraphLineIdToLabel(GraphLineId id) {
    if (id == GraphLineId::None) return nullptr;
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= static_cast<int>(GraphLineId::COUNT)) return nullptr;
    return kGraphLineLabels[idx];
}

// ============================================================================
// Graph definitions
// ============================================================================

// WaveParams and kGraphSamples/kGraphXMax are defined in views/view_common.h

struct GraphLineDef {
    GraphLineId id;
    ImU32       color;
    WaveParams  wave;
};

struct GraphDef {
    GraphId      id;         // unique graph identifier
    ViewId       view;       // which view this graph belongs to
    const char*  section;    // section header, or nullptr for none
    const char*  xLabel;
    const char*  yLabel;
    float        yMin, yMax;
    GraphLineDef lines[2];
};

//                       base    xf1   tf1    a1     p1      xf2    tf2    a2     p2
inline const GraphDef kGraphs[] = {
    { GraphId::Telemetry,    ViewId::Tactical, "MAG FIELD (nT)", "TIME", "nT", -1.5f, 1.5f,
      {{ GraphLineId::MagFieldBx,     kOrange, {   0.0f, 1.0f, 1.0f, 0.5f,  0.0f,   2.3f,  1.7f, 0.5f,  0.0f  }},
       { GraphLineId::MagFieldBy,     kBlue,   {   0.0f, 0.7f, 0.8f, 0.8f,  1.571f, 0.0f,  0.0f, 0.0f,  0.0f  }}} },

    { GraphId::EnvironTrend, ViewId::Environ,  nullptr,            "TIME",      "VALUE",      18.0f, 55.0f,
      {{ GraphLineId::Temperature,    kOrange, {  22.0f, 0.5f, 0.3f, 0.5f,  0.0f,   0.0f,  0.0f, 0.0f,  0.0f  }},
       { GraphLineId::Humidity,       kBlue,   {  47.0f, 0.3f, 0.2f, 2.0f,  1.571f, 0.0f,  0.0f, 0.0f,  0.0f  }}} },

    { GraphId::PowerTrend,   ViewId::Power,    nullptr,            "TIME",      "MW",       1100.0f, 1800.0f,
      {{ GraphLineId::GridLoad,       kOrange, {1300.0f, 0.8f, 0.4f, 50.0f, 0.0f,   0.0f,  0.0f, 0.0f,  0.0f  }},
       { GraphLineId::Generation,     kBlue,   {1696.0f, 0.6f, 0.3f, 30.0f, 1.571f, 0.0f,  0.0f, 0.0f,  0.0f  }}} },

    // RADIO graphs are now defined in views/view_radio.h
};

inline constexpr int kNumGraphs = sizeof(kGraphs) / sizeof(kGraphs[0]);

// ============================================================================
// Shape IDs — unified enum for all SVG shape IDs across all assets
// ============================================================================

enum class ShapeId : int {
    None = -1,

    // ship_wireframe.svg
    Ship_Shield,
    Ship_Saucer,
    Ship_SaucerGrid,
    Ship_Bridge,
    Ship_Neck,
    Ship_EngineeringHull,
    Ship_Deflector,
    Ship_PylonLeft,
    Ship_PylonRight,
    Ship_NacelleLeft,
    Ship_NacelleRight,
    Ship_BussardLeft,
    Ship_BussardRight,
    Ship_NacelleGlowLeft,
    Ship_NacelleGlowRight,

    // goes_orbit.svg
    Map_GeoOrbit,
    Map_Earth,
    Map_EarthGrid,
    Map_Equator,
    Map_GroundStation,
    Map_Goes16Marker,
    Map_Goes16,
    Map_LinkLine,
    Map_SignalCone,
    Map_SubsatellitePoint,
    Map_CoverageArc,
    Map_LonGrid,
    Map_WestLabel,
    Map_EastLabel,

    // rf_circuit.svg
    Rf_Pwr5v1,
    Rf_OpampU4,
    Rf_MixerU2,
    Rf_Gnd12,
    Rf_OscillatorY1,
    Rf_Pwr5v2,
    Rf_MixerU3,
    Rf_DacU1,
    Rf_Gnd8,
    Rf_Gnd7,
    Rf_Pwr5v4,
    Rf_Gnd5,
    Rf_Gnd3,
    Rf_OscillatorY2,
    Rf_AntennaAe1,
    Rf_LabelIfFreqX,

    COUNT
};

inline constexpr int kShipShapeStart = static_cast<int>(ShapeId::Ship_Shield);
inline constexpr int kMapShapeStart  = static_cast<int>(ShapeId::Map_GeoOrbit);
inline constexpr int kRfShapeStart   = static_cast<int>(ShapeId::Rf_Pwr5v1);

inline const char* ShapeIdToString(ShapeId id) {
    if (id == ShapeId::None) return nullptr;
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= static_cast<int>(ShapeId::COUNT)) return nullptr;

    if (idx < kMapShapeStart)
        return kShipWireframeIdStrings[idx - kShipShapeStart];
    if (idx < kRfShapeStart)
        return kGoesOrbitIdStrings[idx - kMapShapeStart];
    return kRfCircuitIdStrings[idx - kRfShapeStart];
}

// ============================================================================
// View type and animation definitions
// ============================================================================

// AnimType, OptionDef, OptionGroupDef are defined in views/view_common.h

enum class ViewType {
    FieldsAndGraphs,  // Standard view: DrawFieldsForView + DrawGraphsForView
    Svg,              // SVG view: load SVG, apply colors/animations, draw
};

struct ShapeAnimDef {
    ShapeId     shapeId;
    AnimType    type;
    float       frequency;   // cycles per second
    ImU32       color1;      // Blink: first color; AlphaPulse: base RGB
    ImU32       color2;      // Blink: second color; AlphaPulse: ignored
    int         minAlpha;    // AlphaPulse: minimum alpha (0-255)
};

struct GlowOverlayDef {
    ShapeId     shapeId;     // Shape to get bounds from (ShapeId::None = no glow)
    float       frequency;   // Pulse frequency
    int         maxAlpha;    // Maximum glow alpha
    ImU32       color;       // RGB color (alpha computed at runtime)
};

struct ShapeColorDef {
    ShapeId     shapeId;
    ImU32       color;
};

// ============================================================================
// Unified view definition
// ============================================================================

struct ViewDef {
    ViewId         id;
    const char*    name;
    ImU32          buttonColor;
    ViewType       type;

    // For Svg views:
    const char*    svgPath;              // path to SVG file
    ImU32          svgStrokeColor;       // default stroke color
    float          svgStrokeWidth;       // stroke width
    ShapeColorDef  defaultColors[16];    // initial shape colors
    int            defaultColorCount;
    ShapeAnimDef   animations[8];        // shape animations
    int            animationCount;
    GlowOverlayDef glowOverlay;          // optional glow effect (shapeId=nullptr if none)

    // Common:
    OptionGroupDef optionGroups[4];
    int            optionGroupCount;
};

// Helper color for detail/grid elements
inline constexpr ImU32 kDetail = IM_COL32(0x66, 0x88, 0xAA, 0xFF);

// ============================================================================
// View definitions are now in individual view headers:
//   - views/view_tactical.h
//   - views/view_ship.h
//   - views/view_environ.h
//   - views/view_power.h
//   - views/view_radio.h
//   - views/view_map.h
//   - views/view_rf.h
// ============================================================================

// ============================================================================
// SVG data-binding definitions — connect fields to SVG shapes
// ============================================================================

// LabelAnchor and ColorDir are defined in views/view_common.h

struct SvgBindingDef {
    FieldId      field;
    ViewId       view;         // which view this binding belongs to
    ShapeId      shapeId;      // SVG shape ID
    LabelAnchor  anchor;
    const char*  label;        // overlay label (null = use FieldDef::label)
    const char*  valueFmt;     // printf format for numeric value (null = use GetString)
    ImU32        normalColor;  // shape color when normal (0 = skip color modulation)
    ColorDir     colorDir;
    double       warnThresh;   // threshold for warning color (kOrange)
    double       critThresh;   // threshold for critical color (kRed)
};

// SVG bindings for views still using the legacy pattern
// SHIP view bindings are now in views/view_ship.h
// MAP view bindings are now in views/view_map.h
// RF view bindings are now in views/view_rf.h
inline const SvgBindingDef kSvgBindings[] = {
    // Placeholder entry to keep array non-empty (required for span iteration)
    {FieldId::MissionElapsed, ViewId::Info, ShapeId::None, LabelAnchor::Center, nullptr, nullptr, 0, ColorDir::LowIsWorse, 0, 0},
};
inline constexpr int kNumSvgBindings = sizeof(kSvgBindings) / sizeof(kSvgBindings[0]);

// ============================================================================
// Field threshold definitions — color scalar fields based on value thresholds
// ============================================================================

struct FieldThresholdDef {
    FieldId   field;
    ColorDir  colorDir;
    double    warnThresh;
    double    critThresh;
};

//                                    FIELD                    DIR                   WARN    CRIT
inline const FieldThresholdDef kFieldThresholds[] = {
    { FieldId::SignalStrength, ColorDir::LowIsWorse,  -50.0,  -60.0 },
    { FieldId::SNR,            ColorDir::LowIsWorse,   60.0,   50.0 },
};
inline constexpr int kNumFieldThresholds = sizeof(kFieldThresholds) / sizeof(kFieldThresholds[0]);
