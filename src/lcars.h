#pragma once

#include "imgui.h"
#include "implot.h"

#include <functional>
#include <vector>

// LCARS color palette
extern const ImU32 kOrange;
extern const ImU32 kPurple;
extern const ImU32 kBlue;
extern const ImU32 kTan;
extern const ImU32 kRed;
extern const ImU32 kBeige;

ImVec4 U32ToVec4(ImU32 c);
void ApplyLCARSTheme();

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
struct LCARSPanel {
    virtual ~LCARSPanel() = default;
    virtual const char* GetTitle() const = 0;
    virtual HOrientation GetHOrientation() const { return HOrientation::Left; }
    virtual const std::vector<PanelView>& GetViews() const = 0;

    int activeView = 0;
};

// Global chrome
void DrawGlobalTopBar(float W);
void DrawGlobalBottomBar(float W, float H);

// Per-panel chrome. Returns content rect as ImVec4(x, y, w, h).
ImVec4 DrawPanelChrome(LCARSPanel& panel, float px, float py, float pw, float ph, VOrientation vOrient);
