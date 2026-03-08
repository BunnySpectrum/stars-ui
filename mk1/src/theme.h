#pragma once

#include "imgui/imgui.h"
#include "implot/implot.h"

// STARS color palette
inline constexpr ImU32 kOrange = IM_COL32(0xFF, 0x99, 0x33, 0xFF);
inline constexpr ImU32 kPurple = IM_COL32(0xCC, 0x99, 0xCC, 0xFF);
inline constexpr ImU32 kBlue   = IM_COL32(0x99, 0x99, 0xFF, 0xFF);
inline constexpr ImU32 kTan    = IM_COL32(0xFF, 0xCC, 0x99, 0xFF);
inline constexpr ImU32 kRed    = IM_COL32(0xCC, 0x66, 0x66, 0xFF);
inline constexpr ImU32 kBeige  = IM_COL32(0xFF, 0xDD, 0xBB, 0xFF);

// Helper functions
ImVec4 U32ToVec4(ImU32 c);
void ApplySTARSTheme();

// Font management
void SetAurebeshFont(ImFont* font);
ImFont* GetAurebeshFont();
