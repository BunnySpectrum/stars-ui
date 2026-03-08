#pragma once

#include "panel_chrome.h"
#include <string>
#include <vector>

// Panel sizing mode
enum class PanelSizing { Auto, Fixed };

// Orientation config from TOML
struct OrientationDef {
    HOrientation horizontal = HOrientation::Left;
    VOrientation vertical = VOrientation::Top;
};

// Widget definition
struct WidgetDef {
    std::string type;           // "stat", "table", "line_chart", etc.
    std::string bind;           // JSONPath: "$.cores[0].load"
    std::string visible_if;     // Optional: JSONPath that must be truthy
    // TODO: nlohmann::json config for widget-specific config in Phase 3
};

// View definition
struct ViewDef {
    std::string id;
    std::string name;           // Display name for view button
    std::vector<WidgetDef> widgets;
    // TODO: std::vector<OptionDef> options in future phases
};

// Panel definition
struct PanelDef {
    std::string id;
    std::string title;
    std::string socket_id;      // Which socket feeds this panel
    OrientationDef orientation;
    PanelSizing sizing = PanelSizing::Auto;
    float fixed_height = 0.0f;  // Only used if sizing == Fixed
    std::vector<ViewDef> views;
    int active_view = 0;
};

// Socket definition
struct SocketDef {
    std::string id;
    std::string path;           // Unix socket path
};

// Screen model (root)
struct ScreenModel {
    std::string title;
    std::vector<SocketDef> sockets;
    std::vector<PanelDef> panels;
};
