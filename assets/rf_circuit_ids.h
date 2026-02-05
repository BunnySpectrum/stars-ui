#pragma once

// Auto-generated from rf_circuit.svg
// Run: python scripts/svg_to_enum.py assets/rf_circuit.svg

enum class RfCircuitId {
    Pwr5v1,
    OpampU4,
    MixerU2,
    Gnd12,
    OscillatorY1,
    Pwr5v2,
    MixerU3,
    DacU1,
    Gnd8,
    Gnd7,
    Pwr5v4,
    Gnd5,
    Gnd3,
    OscillatorY2,
    AntennaAe1,
    LabelIfFreqX,

    COUNT
};

// String table for RfCircuitId
inline const char* const kRfCircuitIdStrings[] = {
    "pwr-5v-1",
    "opamp-u4",
    "mixer-u2",
    "gnd-12",
    "oscillator-y1",
    "pwr-5v-2",
    "mixer-u3",
    "dac-u1",
    "gnd-8",
    "gnd-7",
    "pwr-5v-4",
    "gnd-5",
    "gnd-3",
    "oscillator-y2",
    "antenna-ae1",
    "label-if-freq-x",
};

inline const char* RfCircuitIdToString(RfCircuitId id) {
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= static_cast<int>(RfCircuitId::COUNT)) return nullptr;
    return kRfCircuitIdStrings[idx];
}
