#pragma once

#include "imgui.h"

// Generic rendering driven by the kFields[] and kGraphs[] tables.
// Call these from a PanelView's drawContent lambda.

void DrawFieldsForView(const char* viewName);
void DrawGraphsForView(const char* viewName);

// SVG data-binding helpers — drive shape colors and overlay labels from field data.
class SvgRenderer;

void ApplySvgBindingColors(SvgRenderer& svg, const char* viewName);
void DrawSvgBindingLabels(const SvgRenderer& svg, const char* viewName,
                          ImVec2 origin, ImVec2 size);
