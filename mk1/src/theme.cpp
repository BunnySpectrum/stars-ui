#include "theme.h"

static ImFont* g_aurebeshFont = nullptr;

void SetAurebeshFont(ImFont* font) {
    g_aurebeshFont = font;
}

ImFont* GetAurebeshFont() {
    return g_aurebeshFont;
}

ImVec4 U32ToVec4(ImU32 c) {
    return ImVec4(
        ((c >>  0) & 0xFF) / 255.0f,
        ((c >>  8) & 0xFF) / 255.0f,
        ((c >> 16) & 0xFF) / 255.0f,
        ((c >> 24) & 0xFF) / 255.0f
    );
}

void ApplySTARSTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding    = 0.0f;
    style.FrameRounding     = 8.0f;
    style.GrabRounding      = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.WindowBorderSize  = 0.0f;
    style.FrameBorderSize   = 0.0f;
    style.WindowPadding     = ImVec2(12, 12);
    style.FramePadding      = ImVec2(8, 4);
    style.ItemSpacing       = ImVec2(8, 6);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                  = U32ToVec4(kOrange);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.40f, 0.20f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.02f, 0.02f, 0.04f, 1.00f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.04f, 0.04f, 0.08f, 1.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.05f, 0.05f, 0.10f, 0.95f);
    colors[ImGuiCol_Border]                = ImVec4(0.30f, 0.25f, 0.15f, 0.50f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.08f, 0.06f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.15f, 0.12f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.20f, 0.15f, 0.25f, 1.00f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.06f, 0.04f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.10f, 0.06f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.04f, 0.02f, 0.06f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.04f, 0.50f);
    colors[ImGuiCol_ScrollbarGrab]         = U32ToVec4(kPurple);
    colors[ImGuiCol_ScrollbarGrabHovered]  = U32ToVec4(kBlue);
    colors[ImGuiCol_ScrollbarGrabActive]   = U32ToVec4(kOrange);
    colors[ImGuiCol_CheckMark]             = U32ToVec4(kOrange);
    colors[ImGuiCol_SliderGrab]            = U32ToVec4(kPurple);
    colors[ImGuiCol_SliderGrabActive]      = U32ToVec4(kOrange);
    colors[ImGuiCol_Button]                = ImVec4(0.15f, 0.10f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered]         = U32ToVec4(kPurple);
    colors[ImGuiCol_ButtonActive]          = U32ToVec4(kOrange);
    colors[ImGuiCol_Header]                = ImVec4(0.15f, 0.10f, 0.20f, 1.00f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.25f, 0.18f, 0.30f, 1.00f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.30f, 0.20f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotLines]             = U32ToVec4(kOrange);
    colors[ImGuiCol_PlotLinesHovered]      = U32ToVec4(kTan);
    colors[ImGuiCol_PlotHistogram]         = U32ToVec4(kBlue);
    colors[ImGuiCol_PlotHistogramHovered]  = U32ToVec4(kPurple);

    ImPlotStyle& pstyle = ImPlot::GetStyle();
    pstyle.PlotDefaultSize  = ImVec2(400, 200);
    pstyle.LineWeight       = 2.0f;
    pstyle.PlotPadding      = ImVec2(12, 12);

    ImVec4* pcolors = pstyle.Colors;
    pcolors[ImPlotCol_PlotBg]    = ImVec4(0.02f, 0.02f, 0.06f, 1.00f);
    pcolors[ImPlotCol_PlotBorder]= ImVec4(0.30f, 0.25f, 0.15f, 0.50f);
    pcolors[ImPlotCol_AxisText]  = U32ToVec4(kTan);
    pcolors[ImPlotCol_AxisGrid]  = ImVec4(0.15f, 0.12f, 0.10f, 0.40f);
    pcolors[ImPlotCol_LegendBg]  = ImVec4(0.04f, 0.04f, 0.08f, 0.90f);
    pcolors[ImPlotCol_LegendText]= U32ToVec4(kOrange);
}
