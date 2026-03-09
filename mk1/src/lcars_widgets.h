#pragma once

#include "imgui/imgui.h"

// Elbow corner orientation for LCARS-style L-shaped elbows
enum class ElbowCorner
{
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

// Draw an LCARS elbow using the overdraw method (rectangle + circle cutout)
// x, y: position of the reference point (varies by corner)
// corner: which corner gets the rounded arc cutout
// color: fill color for the elbow
// drawDebugVertices: if true, draws yellow dots at vertices and red dot at arc center
void DrawElbowOverdraw(
    ImDrawList* dl,
    float x, float y,
    ElbowCorner corner,
    ImU32 color,
    bool drawDebugVertices = false);
