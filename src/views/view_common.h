#pragma once

#include "../stars.h"
#include <functional>
#include <span>

// Forward declarations
struct GraphDef;

// ============================================================================
// Common types shared across all view definitions
// ============================================================================

// Display types — tells the update function how to format each field's value.
enum class Display {
    Scalar,     // pre-formatted string (number + units, status text, etc.)
    TimerUp,    // seconds formatted as "DDDD HHh MMm SSs"
    TimerDown,  // seconds counting down, clamped to 0
    Clock,      // time_t formatted as "YYYY.MM.DD  HH:MM:SS"
};

// Animation types for SVG shapes
enum class AnimType {
    Blink,       // Toggle between color1 and color2 at frequency
    AlphaPulse,  // Sin-wave alpha variation (minAlpha to 255)
};

// Text label anchor positions relative to SVG shapes
enum class LabelAnchor { Center, Above, Below, Left, Right };

// Direction for threshold color evaluation
enum class ColorDir { HighIsWorse, LowIsWorse };

// Graph constants
inline constexpr int   kGraphSamples = 256;
inline constexpr float kGraphXMax    = 10.0f;

// Wave parameters for procedural graph data generation:
//   value = base + sin(x*xf1 + t*tf1 + p1)*a1 + sin(x*xf2 + t*tf2 + p2)*a2
struct WaveParams {
    float base;
    float xf1, tf1, a1, p1;
    float xf2, tf2, a2, p2;
};

// Option button definition for view option groups
struct OptionDef {
    const char* label;
    ImU32       color;
};

// Group of option buttons
struct OptionGroupDef {
    OptionDef options[4];  // max 4 options per group
    int       count;
};

// ============================================================================
// ViewContentDef — unified container for view content (fields, graphs, SVG)
// Templated on field def type since each view can have its own field structure.
// ============================================================================

// SVG content definition (optional, for SVG-based views)
struct SvgContentDef {
    const char* path;
    ImU32       strokeColor;
    float       strokeWidth;
    // Note: defaultColors, animations, glowOverlay could be added as spans
};

// Unified view content: fields + graphs + optional SVG
// Use empty spans for missing content, nullptr for no SVG.
template<typename FieldDefT>
struct ViewContentDef {
    std::span<const FieldDefT> fields = {};     // empty if no fields
    std::span<const GraphDef> graphs = {};      // empty if no graphs
    const SvgContentDef* svg = nullptr;         // nullptr if no SVG
};

// ============================================================================
// ViewInfo — exported by each view header, collected by the panel
// ============================================================================

class SvgRenderer;

struct ViewInfo {
    const char*           name;
    ImU32                 buttonColor;
    std::function<void()> drawContent;                  // callable for drawing content
    std::function<void(SvgRenderer&)> initSvg = {};     // optional SVG initializer
    OptionGroupDef        optionGroups[4];
    int                   optionGroupCount;
};
