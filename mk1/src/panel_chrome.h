#pragma once

#include "imgui/imgui.h"
#include <vector>

// Panel orientation
enum class HOrientation { Left, Right };
enum class VOrientation { Top, Bottom };

// View button definition
struct ViewButtonDef {
    const char* name;
    ImU32 color;
};

// Option button definition
struct OptionButtonDef {
    const char* label;
    ImU32 color;
};

// Group of option buttons (separated by dividers)
struct OptionGroupDef {
    std::vector<OptionButtonDef> buttons;
};

// Global chrome
void DrawGlobalTopBar(float W);
void DrawGlobalBottomBar(float W, float H);

// Per-panel chrome. Returns content rect as ImVec4(x, y, w, h).
ImVec4 DrawPanelChrome(
    const char* title,
    float px, float py, float pw, float ph,
    HOrientation hOrient,
    VOrientation vOrient,
    const std::vector<ViewButtonDef>& views,
    int& activeView,
    const std::vector<OptionGroupDef>& optionGroups
);
