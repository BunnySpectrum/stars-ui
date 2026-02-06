#pragma once

// Auto-generated from goes_orbit.svg
// Run: python scripts/svg_to_enum.py assets/goes_orbit.svg

enum class GoesOrbitId {
    GeoOrbit,
    Earth,
    EarthGrid,
    Equator,
    GroundStation,
    Goes16Marker,
    Goes16,
    LinkLine,
    SignalCone,
    SubsatellitePoint,
    CoverageArc,
    LonGrid,
    WestLabel,
    EastLabel,

    COUNT
};

// String table for GoesOrbitId
inline const char* const kGoesOrbitIdStrings[] = {
    "geo-orbit",
    "earth",
    "earth-grid",
    "equator",
    "ground-station",
    "goes16-marker",
    "goes16",
    "link-line",
    "signal-cone",
    "subsatellite-point",
    "coverage-arc",
    "lon-grid",
    "west-label",
    "east-label",
};

inline const char* GoesOrbitIdToString(GoesOrbitId id) {
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= static_cast<int>(GoesOrbitId::COUNT)) return nullptr;
    return kGoesOrbitIdStrings[idx];
}
