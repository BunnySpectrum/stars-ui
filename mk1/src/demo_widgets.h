#pragma once

#include "imgui/imgui.h"

// Section header (tan SeparatorText)
void DrawSectionHeader(const char* text);

// Label/value pair: purple label, beige value, 2-column
void DrawLabelValue(const char* label, const char* value);

// Label/value with threshold coloring: beige (normal), orange (warn), red (crit)
void DrawLabelValueThreshold(const char* label, const char* value, float numericVal, float warn, float crit);

// Progress bar with threshold coloring
void DrawProgressBar(const char* label, float value, float warn, float crit);

// LCARS-styled data table with hardcoded demo data
void DrawDemoTable();

// ImPlot line chart with animated sine waves (orange + blue)
void DrawDemoChart();

// Color palette swatches showing all 6 LCARS colors
void DrawColorPalette();

// Scrolling log display (orange text on black)
void DrawDemoLog();
