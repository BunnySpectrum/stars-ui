#pragma once

// Generic rendering driven by the kFields[] and kGraphs[] tables.
// Call these from a PanelView's drawContent lambda.

void DrawFieldsForView(const char* viewName);
void DrawGraphsForView(const char* viewName);
