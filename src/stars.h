#pragma once

#include "imgui.h"
#include "implot.h"

#include <functional>
#include <vector>

// STARS color palette
inline constexpr ImU32 kOrange = IM_COL32(0xFF, 0x99, 0x33, 0xFF);
inline constexpr ImU32 kPurple = IM_COL32(0xCC, 0x99, 0xCC, 0xFF);
inline constexpr ImU32 kBlue   = IM_COL32(0x99, 0x99, 0xFF, 0xFF);
inline constexpr ImU32 kTan    = IM_COL32(0xFF, 0xCC, 0x99, 0xFF);
inline constexpr ImU32 kRed    = IM_COL32(0xCC, 0x66, 0x66, 0xFF);
inline constexpr ImU32 kBeige  = IM_COL32(0xFF, 0xDD, 0xBB, 0xFF);

ImVec4 U32ToVec4(ImU32 c);
void ApplySTARSTheme();

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
    std::function<void()> drawContent;      // called inside ImGui content window
    std::vector<OptionGroup> optionGroups;  // options for this view
};

// Abstract panel base
struct STARSPanel {
    virtual ~STARSPanel() = default;
    virtual const char* GetTitle() const = 0;
    virtual HOrientation GetHOrientation() const { return HOrientation::Left; }
    virtual float GetHeightWeight() const { return 1.0f; }
    virtual const std::vector<PanelView>& GetViews() const = 0;

    int activeView = 0;
};

// Global chrome
void DrawGlobalTopBar(float W);
void DrawGlobalBottomBar(float W, float H);

// Per-panel chrome. Returns content rect as ImVec4(x, y, w, h).
ImVec4 DrawPanelChrome(STARSPanel& panel, float px, float py, float pw, float ph, VOrientation vOrient);
