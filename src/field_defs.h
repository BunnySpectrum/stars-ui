#pragma once

#include "stars.h"
#include "views/view_common.h"

#include <string>

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
// Field IDs — unified enum for all fields across all views.
// Per-view field tables are defined in views/view_*.h.
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
// Per-view field definition template — parameterized on local ID and base FieldId
// ============================================================================

template<typename LocalIdT, FieldId BaseFieldId>
struct FieldDefT {
    LocalIdT     id;
    const char*  label;
    const char*  units;
    Display      display;
    const char*  section;
    int          column;
    ColorDir     colorDir   = ColorDir::None;
    double       warnThresh = 0.0;
    double       critThresh = 0.0;

    FieldId GetFieldId() const {
        return static_cast<FieldId>(
            static_cast<int>(BaseFieldId) + static_cast<int>(id)
        );
    }
};

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

// Graph definitions are now in individual view headers:
//   - views/view_tactical.h (kTacticalContent.graphs)
//   - views/view_environ.h  (kEnvironContent.graphs)
//   - views/view_power.h    (kPowerContent.graphs)
//   - views/view_radio.h    (kRadioContent.graphs)

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
    FieldsAndGraphs,  // Standard view: uses ViewContentDef with fields + graphs
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

// Helper color for detail/grid elements
inline constexpr ImU32 kDetail = IM_COL32(0x66, 0x88, 0xAA, 0xFF);

// ============================================================================
// SVG data-binding definitions — connect fields to SVG shapes
// ============================================================================

// LabelAnchor and ColorDir are defined in views/view_common.h

struct SvgBindingDef {
    FieldId      field;
    ShapeId      shapeId;      // SVG shape ID
    LabelAnchor  anchor;
    std::string  label;        // overlay label (empty = use FieldDef::label)
    const char*  valueFmt;     // printf format for numeric value (null = use GetString)
    ImU32        normalColor;  // shape color when normal (0 = skip color modulation)
    ColorDir     colorDir;
    double       warnThresh;   // threshold for warning color (kOrange)
    double       critThresh;   // threshold for critical color (kRed)
};

