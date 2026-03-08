# MK1: Socket-Driven Layout Engine for STARS

A data-driven ImGui renderer that loads UI layouts from TOML files and binds them to data streams over sockets. The compiled application remains static; all layout and data binding is defined externally.

## Goals

1. **Separation of concerns**: Compiled C++ handles rendering and socket I/O; TOML files define layout and data bindings
2. **Hot reload**: Modify layout files while running; changes apply without restart
3. **Multi-panel support**: Each panel can bind to a different socket/data source
4. **STARS chrome**: Preserve the existing panel architecture (elbows, view columns, option buttons, color palette)
5. **Core widget set**: Start small, expand as needed

## Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                           MK1 Application                           │
│                                                                     │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌────────────┐  │
│  │ FileWatcher │  │ SocketMgr  │  │  DataStore  │  │  Renderer  │  │
│  │             │  │             │  │             │  │            │  │
│  │ Monitors    │  │ Polls N     │  │ JSON docs   │  │ STARS      │  │
│  │ TOML files  │  │ sockets     │  │ per socket  │  │ chrome +   │  │
│  │ for changes │  │             │  │             │  │ widgets    │  │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘  └─────┬──────┘  │
│         │                │                │                │        │
│         └────────────────┴────────────────┴────────────────┘        │
│                                   │                                 │
│                          ┌────────▼────────┐                        │
│                          │   ScreenModel   │                        │
│                          │                 │                        │
│                          │ - panels[]      │                        │
│                          │ - sockets[]     │                        │
│                          │ - globalBars    │                        │
│                          └─────────────────┘                        │
└─────────────────────────────────────────────────────────────────────┘
```

## Data Flow

```
                    ┌─────────────┐
                    │ TOML Layout │
                    │    File     │
                    └──────┬──────┘
                           │ parse (+ hot reload on change)
                           ▼
┌──────────────┐    ┌─────────────┐    ┌─────────────┐
│ Data Source  │───▶│   Socket    │───▶│  DataStore  │
│ (C++ client) │    │  (JSON rx)  │    │  (per sock) │
└──────────────┘    └─────────────┘    └──────┬──────┘
                                              │ resolve bindings
                                              ▼
                                       ┌─────────────┐
                                       │   Render    │
                                       │   Frame     │
                                       └─────────────┘
```

## File Structure

```
stars-ui/mk1/
├── PLAN.md                 # This document
├── BUILD.bazel             # Bazel build config
├── layouts/                # Layout definitions (hot-reloadable)
│   ├── chrome.toml         # Global chrome configuration
│   ├── demo.toml           # Demo screen layout
│   ├── tactical.toml       # Example multi-panel screen
│   └── panels/             # Individual panel definitions
│       ├── cpu.toml
│       └── sensors.toml
├── src/
│   ├── main.cpp            # App bootstrap, main loop
│   ├── screen_model.h      # ScreenModel, Panel, View, Widget structs
│   ├── layout_loader.h     # TOML → ScreenModel parser
│   ├── layout_loader.cpp
│   ├── socket_manager.h    # Multi-socket management
│   ├── socket_manager.cpp
│   ├── data_store.h        # JSON document storage + path queries
│   ├── data_store.cpp
│   ├── file_watcher.h      # Filesystem change detection
│   ├── file_watcher.cpp
│   ├── widget_registry.h   # Widget type → render function mapping
│   ├── widget_registry.cpp
│   ├── widgets/            # Individual widget implementations
│   │   ├── stat.cpp
│   │   ├── progress_bar.cpp
│   │   ├── table.cpp
│   │   ├── line_chart.cpp
│   │   └── svg.cpp
│   ├── panel_chrome.h      # STARS panel frame rendering
│   ├── panel_chrome.cpp    # (based on poc/src/stars.cpp)
│   └── theme.h             # Color palette, style constants
└── assets/                 # SVGs, fonts (or symlink to parent)
```

## Chrome Reference

Panel chrome implementation is based on `poc/src/stars.cpp` and `poc/src/stars.h`.

### Color Palette

```cpp
// From poc/src/stars.h
inline constexpr ImU32 kOrange = IM_COL32(0xFF, 0x99, 0x33, 0xFF);
inline constexpr ImU32 kPurple = IM_COL32(0xCC, 0x99, 0xCC, 0xFF);
inline constexpr ImU32 kBlue   = IM_COL32(0x99, 0x99, 0xFF, 0xFF);
inline constexpr ImU32 kTan    = IM_COL32(0xFF, 0xCC, 0x99, 0xFF);
inline constexpr ImU32 kRed    = IM_COL32(0xCC, 0x66, 0x66, 0xFF);
inline constexpr ImU32 kBeige  = IM_COL32(0xFF, 0xDD, 0xBB, 0xFF);
```

### Fixed Dimensions (from poc)

| Element | Value |
|---------|-------|
| Global top bar height | 40px |
| Global bottom bar height | 30px |
| Title row height | 30px |
| View column width | 120px |
| Option column width | 120px |
| Elbow radius | 40px |
| Gap between elements | 6px |
| Button gap | 4px |

### Key Functions to Port

| Function | Purpose |
|----------|---------|
| `ApplySTARSTheme()` | ImGui + ImPlot style setup |
| `DrawGlobalTopBar()` | Orange bar with rounded right end |
| `DrawGlobalBottomBar()` | Tan bar with rounded left end |
| `DrawPanelChrome()` | Elbow, title bar, view buttons, option buttons |
| `DrawElbowArc()` | Quarter-circle cutout for elbows |

## Dependencies

From parent stars-ui project (via Bazel):
- ImGui + GLFW/OpenGL backend
- ImPlot (for charts)
- nanosvg (for SVG rendering)
- nlohmann/json (for data parsing)

New dependencies:
- **toml++** — TOML parsing (header-only, C++17)
- Filesystem watching: `kqueue` on macOS, `inotify` on Linux (or poll file mtime)

## Core Data Structures

### ScreenModel

```cpp
struct WidgetDef {
    std::string type;           // "stat", "table", "line_chart", etc.
    std::string bind;           // JSONPath: "$.cores[0].load"
    std::string visible_if;     // Optional: JSONPath that must be truthy
    nlohmann::json config;      // Widget-specific config (thresholds, columns, etc.)
};

struct ViewDef {
    std::string id;
    std::string name;           // Display name for view button
    std::vector<WidgetDef> widgets;
    std::vector<OptionDef> options;
};

enum class PanelSizing { Auto, Fixed };

struct PanelDef {
    std::string id;
    std::string title;
    std::string socket_id;      // Which socket feeds this panel
    Orientation orientation;    // { horizontal: left|right, vertical: top|bottom }
    PanelSizing sizing = PanelSizing::Auto;  // Default: auto-size
    float fixed_height = 0.0f;  // Only used if sizing == Fixed
    std::vector<ViewDef> views;
    int active_view = 0;
};

struct SocketDef {
    std::string id;
    std::string path;           // Unix socket path
};

struct ScreenModel {
    std::string title;
    std::vector<SocketDef> sockets;
    std::vector<PanelDef> panels;
};

// Connection state for error display
struct SocketState {
    bool connected = false;
    std::chrono::steady_clock::time_point last_received;
    nlohmann::json last_data;   // Preserved for display during disconnection
};
```

### DataStore

```cpp
class DataStore {
public:
    // Update data for a socket (called when new JSON arrives)
    void update(const std::string& socket_id, nlohmann::json data);

    // Resolve a binding path against stored data
    // e.g., resolve("cpu", "$.cores[0].load") → "45%"
    nlohmann::json resolve(const std::string& socket_id, const std::string& path);

    // Check if a visible_if condition is truthy
    bool checkVisibility(const std::string& socket_id, const std::string& path);

    // Get connection state for error display
    const SocketState& getState(const std::string& socket_id) const;

private:
    std::unordered_map<std::string, SocketState> state_;
};
```

## Layout File Structure

MK1 uses a multi-file layout system:

| File | Purpose | Hot Reload |
|------|---------|------------|
| `layouts/chrome.toml` | Global chrome config (colors, dimensions) | TBD |
| `layouts/<screen>.toml` | Screen definition (which panels, sockets) | Yes |
| `layouts/panels/<panel>.toml` | Individual panel layouts | Yes |

### Screen Layout (e.g., `tactical.toml`)

```toml
[screen]
title = "TACTICAL DISPLAY"

[[sockets]]
id = "cpu"
path = "/tmp/cpu-telem.sock"

[[sockets]]
id = "sensors"
path = "/tmp/sensor-telem.sock"

[[panels]]
id = "cpu-panel"
file = "panels/cpu.toml"       # Load panel definition from separate file
socket = "cpu"
orientation = { horizontal = "left", vertical = "bottom" }
sizing = "auto"                # or "fixed" with height = 300

[[panels]]
id = "sensor-panel"
file = "panels/sensors.toml"
socket = "sensors"
orientation = { horizontal = "left", vertical = "top" }
```

### Panel Layout (e.g., `panels/cpu.toml`)

```toml
[panel]
title = "SYSTEM STATUS"

[[views]]
id = "overview"
name = "OVR"

[[views.widgets]]
type = "stat"
label = "CPU LOAD"
bind = "$.load_percent"
unit = "percent"

[[views.widgets]]
type = "progress_bar"
bind = "$.load_value"
thresholds = { warn = 70, crit = 90 }

[[views.widgets]]
type = "table"
bind = "$.cores"
visible_if = "$.show_cores"    # Only show when $.show_cores is truthy
columns = [
  { header = "CORE", field = "id" },
  { header = "TYPE", field = "type" },
  { header = "FREQ", field = "freq", unit = "GHz" },
  { header = "LOAD", field = "load", unit = "percent" }
]

[[views]]
id = "graph"
name = "GRF"

[[views.widgets]]
type = "line_chart"
title = "CPU HISTORY"
bind = "$.history"
series = [
  { field = "user", color = "orange", label = "USER" },
  { field = "system", color = "blue", label = "SYS" }
]
y_axis = { min = 0, max = 100, unit = "percent" }
```

### Chrome Config (e.g., `chrome.toml`)

```toml
[colors]
orange = "#FF9933"
purple = "#CC99CC"
blue = "#9999FF"
tan = "#FFCC99"
red = "#CC6666"
beige = "#FFDDBB"
background = "#000000"
window_bg = "#050508"

[dimensions]
global_top_bar_height = 40
global_bottom_bar_height = 30
title_row_height = 30
view_column_width = 120
option_column_width = 120
elbow_radius = 40
gap = 6
button_gap = 4
```

### Widget Types (Core Set)

| Type | Description | Key Config |
|------|-------------|------------|
| `stat` | Single value display | `label`, `bind`, `unit`, `thresholds` |
| `progress_bar` | Horizontal bar | `bind`, `min`, `max`, `thresholds` |
| `table` | Data table | `bind` (array), `columns[]` |
| `line_chart` | Time series | `bind`, `series[]`, `y_axis` |
| `svg` | SVG with overlays | `asset`, `bindings[]` (shape→data) |

All widgets support `visible_if` for conditional rendering.

## Error Handling & Connection State

When a socket disconnects or stops sending data:

1. **Preserve last data**: Continue displaying the last received values
2. **Panel-level indicator**: Draw error styling around the panel
   - TBD: Bounding box color change (e.g., red/orange border)
   - TBD: Error symbol in title bar or elbow
   - TBD: "DISCONNECTED" or "STALE" label
3. **Per-widget fallback**: Widgets show last value but may dim or show indicator

```cpp
void renderPanelWithState(PanelDef& panel, DataStore& store) {
    auto& state = store.getState(panel.socket_id);

    // Draw error indicator if disconnected
    if (!state.connected) {
        drawDisconnectedOverlay(panel);
    }

    // Widgets still render using state.last_data
    for (auto& widget : currentView.widgets) {
        render_widget(widget, state.last_data);
    }
}
```

## Data Protocol

### Length-Prefixed JSON (Default)

Same as cpu_textual.py uses:
```
[4-byte big-endian length][JSON payload]
```

Simple, proven, works well for continuous streams.

## Hot Reload Implementation

### File Watching

```cpp
class FileWatcher {
public:
    FileWatcher(const std::string& path, std::function<void()> on_change);
    void poll();  // Call each frame

private:
    std::string path_;
    std::filesystem::file_time_type last_modified_;
    std::function<void()> callback_;
};
```

Simple approach: check `std::filesystem::last_write_time()` each frame (or every N frames). If changed, trigger reload callback.

### Reload Strategy

1. Parse new TOML into a temporary `ScreenModel`
2. If parse succeeds:
   - Preserve `active_view` indices from current model where panel IDs match
   - Swap new model into place
   - Log reload success
3. If parse fails:
   - Log error with line number
   - Keep current model running

### Preserving State Across Reload

| State | Preserved | Notes |
|-------|-----------|-------|
| Active view per panel | Yes | Match by panel ID |
| Scroll positions | No | Reset on reload |
| Socket connections | Yes | Sockets only reconnect if path changes |
| Data in DataStore | Yes | Data persists; only bindings re-resolve |

## Implementation Phases

### Phase 1: Skeleton

- [ ] BUILD.bazel linking to parent deps (imgui, implot, nanosvg, nlohmann_json)
- [ ] Add toml++ dependency
- [ ] main.cpp with ImGui bootstrap (adapt from poc/src/main.cpp)
- [ ] Load a minimal TOML file, print parsed structure
- [ ] Single hardcoded panel with STARS chrome (port from poc/src/stars.cpp)

### Phase 2: Data Flow

- [ ] SocketManager: connect to one socket, receive length-prefixed JSON
- [ ] DataStore: store JSON, implement basic path resolution (JSONPath subset)
- [ ] Wire socket → DataStore → simple text widget displaying raw value

### Phase 3: Widget Core

- [ ] WidgetRegistry with type → render function dispatch
- [ ] `stat` widget (label + value + optional unit)
- [ ] `progress_bar` widget (with thresholds for color)
- [ ] `table` widget (bind to array, render columns)

### Phase 4: Hot Reload

- [ ] FileWatcher implementation (mtime polling)
- [ ] LayoutLoader with error handling and line number reporting
- [ ] Reload preserving panel state (active_view)
- [ ] Watch multiple files (screen + panel files)

### Phase 5: Layout Comparison Demos

Build demo layouts to compare widget arrangement approaches:

- [ ] **Flow layout demo**: Widgets stack top-to-bottom, auto-height
- [ ] **Grid layout demo**: Widgets use x/y/w/h in a grid system
- [ ] **Hybrid demo**: Sections flow, widgets within sections use grid

Evaluate:
- Ease of authoring in TOML
- Visual results with varying data sizes (tables with few vs many rows)
- Responsive behavior when window resizes

Document findings and choose approach for Phase 6+.

### Phase 6: Charts & SVG

- [ ] `line_chart` widget (ImPlot integration)
- [ ] `svg` widget (nanosvg + color bindings from data)
- [ ] Threshold-based color for SVG shape bindings

### Phase 7: Conditional Visibility & Error States

- [ ] `visible_if` binding support for widgets
- [ ] Socket connection state tracking
- [ ] Error overlay styling at panel level (TBD: exact visual treatment)
- [ ] Stale data indicator (e.g., after N seconds without update)

### Phase 8: Polish

- [ ] Multiple sockets / multi-panel demo
- [ ] View switching via view buttons (click handling)
- [ ] Option buttons with callbacks
- [ ] Chrome config hot reload (if desired)

## Demo Data Source

For testing, create a simple C++ program that:
1. Listens on a Unix socket
2. Sends periodic JSON updates (CPU-like telemetry)
3. Can be controlled via stdin (inject anomalies, change values, disconnect)

```cpp
// demo_source.cpp - minimal test data generator
// Sends: {"load_percent": "45%", "cores": [...], "history": [...]}
// Commands: "disconnect", "reconnect", "spike", "normal"
```

This mirrors the existing `client_cpu_textual.cpp` pattern from laptop-telem.

## Open Questions (Resolved)

| Question | Decision |
|----------|----------|
| Panel sizing | Allow fixed or auto-size; default to auto-size |
| Grid vs flow | Phase 5 will build demos to compare; decide after evaluation |
| Conditional visibility | Yes, via `visible_if` binding |
| Error display | Show last data with panel-level error styling (TBD exact visuals) |
| Layout files | One per screen + one per panel + chrome config; screen/panel hot-reload |

## Open Questions (Remaining)

1. **Chrome config hot reload**: Should `chrome.toml` changes apply live, or require restart? (TBD)

2. **Grid coordinate system**: If grid is chosen, what's the grid resolution? 12-column? 24-column? Pixel-based?

3. **Stale data threshold**: How long without data before showing "stale" indicator? Configurable per-socket?
