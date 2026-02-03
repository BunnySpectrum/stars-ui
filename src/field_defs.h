#pragma once

#include "lcars.h"

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
// Display types — tells the update function how to format each field's value.
// ============================================================================

enum class Display {
    Scalar,     // pre-formatted string (number + units, status text, etc.)
    TimerUp,    // seconds formatted as "DDDD HHh MMm SSs"
    TimerDown,  // seconds counting down, clamped to 0
    Clock,      // time_t formatted as "YYYY.MM.DD  HH:MM:SS"
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
    const char* view;
    const char* section;
    int         column;
};

//                              ID                      LABEL                   UNITS   DISPLAY            VIEW         SECTION                       COL
inline const FieldDef kFields[] = {
    // INFO panel — left side
    { FieldId::MissionElapsed,  "MISSION ELAPSED",      "",     Display::TimerUp,   "INFO",      "MISSION TIMERS",             0 },
    { FieldId::Uptime,          "UPTIME",               "",     Display::TimerUp,   "INFO",      "MISSION TIMERS",             0 },
    { FieldId::TimeToEclipse,   "TIME TO ECLIPSE",      "",     Display::TimerDown, "INFO",      "MISSION TIMERS",             0 },
    { FieldId::NextMaintenance, "NEXT MAINTENANCE",     "",     Display::TimerDown, "INFO",      "MISSION TIMERS",             0 },
    // INFO panel — right side
    { FieldId::PacificTime,     "PACIFIC TIME",         "",     Display::Clock,     "INFO",      "CHRONOMETER",                1 },
    { FieldId::Utc,             "UTC",                  "",     Display::Clock,     "INFO",      "CHRONOMETER",                1 },
    { FieldId::Stardate,        "STARDATE",             "",     Display::Scalar,    "INFO",      "CHRONOMETER",                1 },

    // TACTICAL panel
    { FieldId::TacWarpCore,     "WARP CORE",            "",     Display::Scalar,    "TACTICAL",  "SYSTEM STATUS",              0 },
    { FieldId::TacShields,      "SHIELDS",              "",     Display::Scalar,    "TACTICAL",  "SYSTEM STATUS",              0 },
    { FieldId::TacHullIntegrity,"HULL INTEGRITY",       "",     Display::Scalar,    "TACTICAL",  "SYSTEM STATUS",              0 },
    { FieldId::TacLifeSupport,  "LIFE SUPPORT",         "",     Display::Scalar,    "TACTICAL",  "SYSTEM STATUS",              0 },

    // ENVIRON — atmospheric
    { FieldId::EnvTemperature,  "TEMPERATURE",          "C",    Display::Scalar,    "ENVIRON",   "ATMOSPHERIC CONDITIONS",     0 },
    { FieldId::EnvHumidity,     "HUMIDITY",             "%",    Display::Scalar,    "ENVIRON",   "ATMOSPHERIC CONDITIONS",     0 },
    { FieldId::EnvPressure,     "PRESSURE",             "kPa",  Display::Scalar,    "ENVIRON",   "ATMOSPHERIC CONDITIONS",     0 },
    { FieldId::EnvAirQuality,   "AIR QUALITY",          "",     Display::Scalar,    "ENVIRON",   "ATMOSPHERIC CONDITIONS",     0 },
    { FieldId::EnvCO2,          "CO2",                  "PPM",  Display::Scalar,    "ENVIRON",   "ATMOSPHERIC CONDITIONS",     0 },
    { FieldId::EnvO2,           "O2",                   "%",    Display::Scalar,    "ENVIRON",   "ATMOSPHERIC CONDITIONS",     0 },
    // ENVIRON — decks
    { FieldId::Deck1Bridge,     "DECK 1 BRIDGE",        "C",    Display::Scalar,    "ENVIRON",   "DECK STATUS",                1 },
    { FieldId::Deck2Officers,   "DECK 2 OFFICERS",      "C",    Display::Scalar,    "ENVIRON",   "DECK STATUS",                1 },
    { FieldId::Deck3Crew,       "DECK 3 CREW",          "C",    Display::Scalar,    "ENVIRON",   "DECK STATUS",                1 },
    { FieldId::Deck5Science,    "DECK 5 SCIENCE",       "C",    Display::Scalar,    "ENVIRON",   "DECK STATUS",                1 },
    { FieldId::Deck7Engineering,"DECK 7 ENGINEERING",   "C",    Display::Scalar,    "ENVIRON",   "DECK STATUS",                1 },
    { FieldId::Deck10Cargo,     "DECK 10 CARGO",        "C",    Display::Scalar,    "ENVIRON",   "DECK STATUS",                1 },

    // POWER — generation
    { FieldId::GenWarpCore,     "WARP CORE",            "MW",   Display::Scalar,    "POWER",     "POWER GENERATION",           0 },
    { FieldId::GenImpulse,      "IMPULSE",              "MW",   Display::Scalar,    "POWER",     "POWER GENERATION",           0 },
    { FieldId::GenSolar,        "SOLAR",                "MW",   Display::Scalar,    "POWER",     "POWER GENERATION",           0 },
    { FieldId::GenAuxFusion,    "AUX FUSION",           "MW",   Display::Scalar,    "POWER",     "POWER GENERATION",           0 },
    { FieldId::GenTotal,        "TOTAL",                "MW",   Display::Scalar,    "POWER",     "POWER GENERATION",           0 },
    // POWER — consumption
    { FieldId::ConPropulsion,   "PROPULSION",           "MW",   Display::Scalar,    "POWER",     "POWER CONSUMPTION",          1 },
    { FieldId::ConShields,      "SHIELDS",              "MW",   Display::Scalar,    "POWER",     "POWER CONSUMPTION",          1 },
    { FieldId::ConLifeSupport,  "LIFE SUPPORT",         "MW",   Display::Scalar,    "POWER",     "POWER CONSUMPTION",          1 },
    { FieldId::ConSensors,      "SENSORS",              "MW",   Display::Scalar,    "POWER",     "POWER CONSUMPTION",          1 },
    { FieldId::ConComputers,    "COMPUTERS",            "MW",   Display::Scalar,    "POWER",     "POWER CONSUMPTION",          1 },
    { FieldId::ConWeapons,      "WEAPONS",              "MW",   Display::Scalar,    "POWER",     "POWER CONSUMPTION",          1 },
    { FieldId::ConTotal,        "TOTAL",                "MW",   Display::Scalar,    "POWER",     "POWER CONSUMPTION",          1 },
    // POWER — reserves
    { FieldId::BatteryA,        "BATTERY A",            "%",    Display::Scalar,    "POWER",     "ENERGY RESERVES",            2 },
    { FieldId::BatteryB,        "BATTERY B",            "%",    Display::Scalar,    "POWER",     "ENERGY RESERVES",            2 },
    { FieldId::Emergency,       "EMERGENCY",            "%",    Display::Scalar,    "POWER",     "ENERGY RESERVES",            2 },
    { FieldId::Surplus,         "SURPLUS",              "MW",   Display::Scalar,    "POWER",     "ENERGY RESERVES",            2 },

    // RADIO — channels
    { FieldId::Ch1Subspace,     "CH 1  SUBSPACE",       "GHz",  Display::Scalar,    "RADIO",     "ACTIVE CHANNELS",            0 },
    { FieldId::Ch2Emergency,    "CH 2  EMERGENCY",      "MHz",  Display::Scalar,    "RADIO",     "ACTIVE CHANNELS",            0 },
    { FieldId::Ch3Tactical,     "CH 3  TACTICAL",       "GHz",  Display::Scalar,    "RADIO",     "ACTIVE CHANNELS",            0 },
    { FieldId::Ch4Science,      "CH 4  SCIENCE",        "GHz",  Display::Scalar,    "RADIO",     "ACTIVE CHANNELS",            0 },
    { FieldId::Ch5Command,      "CH 5  COMMAND",        "GHz",  Display::Scalar,    "RADIO",     "ACTIVE CHANNELS",            0 },
    { FieldId::Ch6Diplomatic,   "CH 6  DIPLOMATIC",     "GHz",  Display::Scalar,    "RADIO",     "ACTIVE CHANNELS",            0 },
    // RADIO — signal
    { FieldId::SubspaceBandwidth,"SUBSPACE BANDWIDTH",  "GHz",  Display::Scalar,    "RADIO",     "SIGNAL STATUS",              1 },
    { FieldId::SignalStrength,  "SIGNAL STRENGTH",      "dBm",  Display::Scalar,    "RADIO",     "SIGNAL STATUS",              1 },
    { FieldId::NoiseFloor,      "NOISE FLOOR",          "dBm",  Display::Scalar,    "RADIO",     "SIGNAL STATUS",              1 },
    { FieldId::SNR,             "SNR",                  "dB",   Display::Scalar,    "RADIO",     "SIGNAL STATUS",              1 },
    { FieldId::AntennaArray,    "ANTENNA ARRAY",        "",     Display::Scalar,    "RADIO",     "SIGNAL STATUS",              1 },
    { FieldId::Range,           "RANGE",                "km",   Display::Scalar,    "RADIO",     "SIGNAL STATUS",              1 },
};

inline constexpr int kNumFields = sizeof(kFields) / sizeof(kFields[0]);

// ============================================================================
// Graph definitions
// ============================================================================

inline constexpr int   kGraphSamples = 256;
inline constexpr float kGraphXMax    = 10.0f;

// Wave parameters for procedural graph data generation:
//   value = base + sin(x*xf1 + t*tf1 + p1)*a1 + sin(x*xf2 + t*tf2 + p2)*a2
struct WaveParams {
    float base;
    float xf1, tf1, a1, p1;
    float xf2, tf2, a2, p2;
};

struct GraphLineDef {
    const char* label;
    ImU32       color;
    WaveParams  wave;
};

struct GraphDef {
    const char*  plotId;     // ImPlot ID, e.g. "##Telemetry"
    const char*  view;       // which view this graph belongs to
    const char*  section;    // section header, or nullptr for none
    const char*  xLabel;
    const char*  yLabel;
    float        yMin, yMax;
    GraphLineDef lines[2];
};

//                                                                                               base    xf1   tf1    a1     p1      xf2    tf2    a2     p2
inline const GraphDef kGraphs[] = {
    { "##Telemetry",    "TACTICAL", "MAG FIELD (nT)", "TIME", "nT", -1.5f, 1.5f,
      {{ "Bx",    kOrange, {   0.0f, 1.0f, 1.0f, 0.5f,  0.0f,   2.3f,  1.7f, 0.5f,  0.0f  }},
       { "By",      kBlue,   {   0.0f, 0.7f, 0.8f, 0.8f,  1.571f, 0.0f,  0.0f, 0.0f,  0.0f  }}} },

    { "##EnvironTrend", "ENVIRON",  nullptr,            "TIME",      "VALUE",      18.0f, 55.0f,
      {{ "Temperature (C)",  kOrange, {  22.0f, 0.5f, 0.3f, 0.5f,  0.0f,   0.0f,  0.0f, 0.0f,  0.0f  }},
       { "Humidity (%)",     kBlue,   {  47.0f, 0.3f, 0.2f, 2.0f,  1.571f, 0.0f,  0.0f, 0.0f,  0.0f  }}} },

    { "##PowerTrend",   "POWER",    nullptr,            "TIME",      "MW",       1100.0f, 1800.0f,
      {{ "Grid Load",        kOrange, {1300.0f, 0.8f, 0.4f, 50.0f, 0.0f,   0.0f,  0.0f, 0.0f,  0.0f  }},
       { "Generation",       kBlue,   {1696.0f, 0.6f, 0.3f, 30.0f, 1.571f, 0.0f,  0.0f, 0.0f,  0.0f  }}} },

    { "##RadioSignal",  "RADIO",    nullptr,            "FREQUENCY", "dBm",     -120.0f, -20.0f,
      {{ "Signal Strength",  kOrange, { -42.0f, 2.0f, 1.5f, 8.0f,  0.0f,   7.3f,  3.1f, 3.0f,  0.0f  }},
       { "Noise",            kPurple, {-110.0f, 3.0f, 0.7f, 2.0f,  0.0f,  11.0f,  2.3f, 1.5f,  1.571f}}} },

    { "##OrbitalTrack", "RADIO",    "ORBITAL TRACK",    "TIME",      "km",   35750.0f, 42000.0f,
      {{ "Altitude",         kOrange, {35786.0f, 0.5f, 0.1f, 10.0f, 0.0f,  0.0f,  0.0f, 0.0f,  0.0f  }},
       { "Range",            kBlue,   {38000.0f, 0.3f, 0.2f, 500.0f, 1.571f, 0.0f,  0.0f, 0.0f,  0.0f  }}} },
};

inline constexpr int kNumGraphs = sizeof(kGraphs) / sizeof(kGraphs[0]);

// ============================================================================
// SVG view definitions — maps view names to SVG file paths
// ============================================================================

struct SvgViewDef {
    const char* view;       // view name (matches PanelView::name)
    const char* svgPath;    // path to SVG file
};

inline const SvgViewDef kSvgViews[] = {
    { "SHIP",  "assets/ship_wireframe.svg" },
    { "MAP",   "assets/goes_orbit.svg" },
};
inline constexpr int kNumSvgViews = sizeof(kSvgViews) / sizeof(kSvgViews[0]);

// ============================================================================
// SVG data-binding definitions — connect fields to SVG shapes
// ============================================================================

enum class LabelAnchor { Center, Above, Below, Left, Right };
enum class ColorDir { HighIsWorse, LowIsWorse };

struct SvgBindingDef {
    FieldId      field;
    const char*  view;         // matches SvgViewDef::view
    const char*  shapeId;      // SVG shape ID
    LabelAnchor  anchor;
    const char*  label;        // overlay label (null = use FieldDef::label)
    const char*  valueFmt;     // printf format for numeric value (null = use GetString)
    ImU32        normalColor;  // shape color when normal (0 = skip color modulation)
    ColorDir     colorDir;
    double       warnThresh;   // threshold for warning color (kOrange)
    double       critThresh;   // threshold for critical color (kRed)
};

//                                     FIELD                       VIEW    SHAPE               ANCHOR               LABEL       FMT       NORMAL   DIR                   WARN   CRIT
inline const SvgBindingDef kSvgBindings[] = {
    // SHIP view bindings
    {FieldId::TacShields,      "SHIP", "shield",           LabelAnchor::Right, "SHIELDS",  "%.0f%%", kBlue,   ColorDir::LowIsWorse, 50.0, 25.0},
    {FieldId::TacHullIntegrity,"SHIP", "engineering-hull", LabelAnchor::Center, "HULL",     "%.1f%%", kBlue,   ColorDir::LowIsWorse, 60.0, 30.0},
    {FieldId::TacWarpCore,     "SHIP", "deflector",        LabelAnchor::Below,  "WARP CORE","%.1f%%", kOrange, ColorDir::LowIsWorse, 50.0, 25.0},
    {FieldId::TacLifeSupport,  "SHIP", "bridge",           LabelAnchor::Above,  "LIFE SUPT","%.0f%%", kOrange, ColorDir::LowIsWorse, 50.0, 25.0},
    // MAP view bindings (GOES orbital map)
    {FieldId::SignalStrength,  "MAP",  "link-line",        LabelAnchor::Left,   "SIGNAL",   "%.0f dBm", kOrange, ColorDir::LowIsWorse, -50.0, -60.0},
    {FieldId::SNR,             "MAP",  "goes16",           LabelAnchor::Above,  "SNR",      "%.0f dB",  kBlue,   ColorDir::LowIsWorse, 60.0, 50.0},
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
