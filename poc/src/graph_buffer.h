#pragma once

// Graph constants - must match view_common.h
inline constexpr int   kGraphSamples = 256;

struct GraphBuffer {
    float xs[kGraphSamples];
    float ys[2][kGraphSamples];
};
