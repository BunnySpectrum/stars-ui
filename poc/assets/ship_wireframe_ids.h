#pragma once

// Auto-generated from ship_wireframe.svg
// Run: python scripts/svg_to_enum.py assets/ship_wireframe.svg

enum class ShipWireframeId {
    Shield,
    Saucer,
    SaucerGrid,
    Bridge,
    Neck,
    EngineeringHull,
    Deflector,
    PylonLeft,
    PylonRight,
    NacelleLeft,
    NacelleRight,
    BussardLeft,
    BussardRight,
    NacelleGlowLeft,
    NacelleGlowRight,

    COUNT
};

// String table for ShipWireframeId
inline const char* const kShipWireframeIdStrings[] = {
    "shield",
    "saucer",
    "saucer-grid",
    "bridge",
    "neck",
    "engineering-hull",
    "deflector",
    "pylon-left",
    "pylon-right",
    "nacelle-left",
    "nacelle-right",
    "bussard-left",
    "bussard-right",
    "nacelle-glow-left",
    "nacelle-glow-right",
};

inline const char* ShipWireframeIdToString(ShipWireframeId id) {
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= static_cast<int>(ShipWireframeId::COUNT)) return nullptr;
    return kShipWireframeIdStrings[idx];
}
