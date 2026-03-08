#pragma once

#include "screen_model.h"
#include <string>
#include <optional>

// Result of loading a layout
struct LayoutLoadResult {
    bool success = false;
    std::string error_message;
    ScreenModel model;
};

// Load a screen layout from a TOML file
LayoutLoadResult LoadLayoutFromFile(const std::string& path);

// Print a parsed ScreenModel to stdout for debugging
void PrintScreenModel(const ScreenModel& model);
