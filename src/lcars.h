#pragma once

#include "imgui.h"
#include "implot.h"

// LCARS color palette
extern const ImU32 kOrange;
extern const ImU32 kPurple;
extern const ImU32 kBlue;
extern const ImU32 kTan;
extern const ImU32 kRed;
extern const ImU32 kBeige;

ImVec4 U32ToVec4(ImU32 c);
void ApplyLCARSTheme();
void DrawLCARSFrame(float W, float H, float dividerY);

struct LCARSPanel {
    virtual ~LCARSPanel() = default;
    virtual const char* GetTitle() const = 0;
    virtual void DrawUpper() = 0;
    virtual void DrawLower() = 0;
};
