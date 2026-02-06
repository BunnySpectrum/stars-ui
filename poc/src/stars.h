#pragma once

#include "imgui/imgui.h"
#include "implot/implot.h"

#include "svg_renderer.h"
#include "graph_buffer.h"

#include <functional>
#include <map>
#include <span>
#include <string>
#include <vector>

// Forward declarations
struct FieldStore;

// STARS color palette
inline constexpr ImU32 kOrange = IM_COL32(0xFF, 0x99, 0x33, 0xFF);
inline constexpr ImU32 kPurple = IM_COL32(0xCC, 0x99, 0xCC, 0xFF);
inline constexpr ImU32 kBlue   = IM_COL32(0x99, 0x99, 0xFF, 0xFF);
inline constexpr ImU32 kTan    = IM_COL32(0xFF, 0xCC, 0x99, 0xFF);
inline constexpr ImU32 kRed    = IM_COL32(0xCC, 0x66, 0x66, 0xFF);
inline constexpr ImU32 kBeige  = IM_COL32(0xFF, 0xDD, 0xBB, 0xFF);

ImVec4 U32ToVec4(ImU32 c);
void ApplySTARSTheme();
void SetAurebeshFont(ImFont* font);
ImFont* GetAurebeshFont();

// Panel orientation
enum class HOrientation { Left, Right };
enum class VOrientation { Top, Bottom };

// Option button in the option column
struct OptionButton {
    const char* label;
    ImU32 color;
    std::function<void()> onClick;
};

// Group of option buttons separated by dividers
struct OptionGroup {
    std::vector<OptionButton> buttons;
};

// A view within a panel
struct PanelView {
    const char* name;                       // shown on elbow + view button
    ImU32 buttonColor;                      // view button fill color
    std::function<void(const FieldStore&, std::span<const GraphBuffer>)> drawContent;  // called inside ImGui content window
    std::vector<OptionGroup> optionGroups;  // options for this view

    // Per-view data
    FieldStore* fields = nullptr;                              // per-view field storage
    std::vector<GraphBuffer> graphBufs;                        // per-view graph buffers
    std::function<void(FieldStore&, std::span<GraphBuffer>)> updateFields;  // update function
};

// Abstract panel base
struct STARSPanel {
    virtual ~STARSPanel() = default;
    const char* GetTitle() const { return title_; }
    virtual HOrientation GetHOrientation() const { return HOrientation::Left; }
    float GetHeightWeight() const { return heightWeight_; }
    std::vector<PanelView>& GetViews() { return views_; }
    const std::vector<PanelView>& GetViews() const { return views_; }

    int activeView = 0;

protected:
    STARSPanel(const char* title, float heightWeight = 1.0f)
        : title_(title), heightWeight_(heightWeight) {}

    std::vector<PanelView> views_;
    std::map<std::string, SvgRenderer> svgRenderers_;

private:
    const char* title_;
    float heightWeight_;
};

// Global chrome
void DrawGlobalTopBar(float W);
void DrawGlobalBottomBar(float W, float H);

// Per-panel chrome. Returns content rect as ImVec4(x, y, w, h).
ImVec4 DrawPanelChrome(STARSPanel& panel, float px, float py, float pw, float ph, VOrientation vOrient);
