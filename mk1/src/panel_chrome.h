#pragma once

#include "imgui/imgui.h"

// Panel orientation
enum class HOrientation { Left, Right };
enum class VOrientation { Top, Bottom };

// Global chrome
void DrawGlobalTopBar(float W);
void DrawGlobalBottomBar(float W, float H);

// Per-panel chrome. Returns content rect as ImVec4(x, y, w, h).
// For Phase 1, simplified to just draw chrome for a hardcoded panel
ImVec4 DrawPanelChrome(
    const char* title,
    const char* viewName,
    float px, float py, float pw, float ph,
    HOrientation hOrient,
    VOrientation vOrient
);
