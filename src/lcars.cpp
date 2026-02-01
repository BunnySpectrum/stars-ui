#include "lcars.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// LCARS color palette
const ImU32 kOrange = IM_COL32(0xFF, 0x99, 0x33, 0xFF);
const ImU32 kPurple = IM_COL32(0xCC, 0x99, 0xCC, 0xFF);
const ImU32 kBlue   = IM_COL32(0x99, 0x99, 0xFF, 0xFF);
const ImU32 kTan    = IM_COL32(0xFF, 0xCC, 0x99, 0xFF);
const ImU32 kRed    = IM_COL32(0xCC, 0x66, 0x66, 0xFF);
const ImU32 kBeige  = IM_COL32(0xFF, 0xDD, 0xBB, 0xFF);

ImVec4 U32ToVec4(ImU32 c) {
    return ImVec4(
        ((c >>  0) & 0xFF) / 255.0f,
        ((c >>  8) & 0xFF) / 255.0f,
        ((c >> 16) & 0xFF) / 255.0f,
        ((c >> 24) & 0xFF) / 255.0f
    );
}

void ApplyLCARSTheme() {
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

// ---------------------------------------------------------------------------
// Global chrome
// ---------------------------------------------------------------------------

void DrawGlobalTopBar(float W) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const float h = 40.0f;
    dl->AddRectFilled(ImVec2(0, 0), ImVec2(W, h), kOrange);
    dl->AddRectFilled(ImVec2(W - 20, 0), ImVec2(W, h), kOrange, 20.0f, ImDrawFlags_RoundCornersRight);
}

void DrawGlobalBottomBar(float W, float H) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const float h = 30.0f;
    const float inset = 6.0f;
    dl->AddRectFilled(ImVec2(inset, H - h), ImVec2(W, H), kTan);
    dl->AddRectFilled(ImVec2(inset, H - h), ImVec2(inset + 20, H), kTan, 20.0f, ImDrawFlags_RoundCornersLeft);
}

// ---------------------------------------------------------------------------
// Helper: quarter-circle arc cutout (filled black)
// ---------------------------------------------------------------------------

static void DrawElbowArc(ImDrawList* dl, float cx, float cy, float r, float arcStart) {
    const int segs = 32;
    ImVector<ImVec2> pts;
    for (int i = 0; i <= segs; i++) {
        float a = arcStart + ((float)M_PI * 0.5f) * ((float)i / (float)segs);
        pts.push_back(ImVec2(cx + cosf(a) * r, cy + sinf(a) * r));
    }
    pts.push_back(ImVec2(cx, cy));
    dl->AddConvexPolyFilled(pts.Data, pts.Size, IM_COL32(0, 0, 0, 255));
}

// ---------------------------------------------------------------------------
// Per-panel chrome
// ---------------------------------------------------------------------------

ImVec4 DrawPanelChrome(LCARSPanel& panel, float px, float py, float pw, float ph, VOrientation vOrient) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    const float titleH = 30.0f;
    const float viewW  = 120.0f;
    const float optW   = 120.0f;
    const float elbowR = 40.0f;
    const float gap    = 6.0f;
    const float btnGap = 4.0f;

    bool viewLeft  = (panel.GetHOrientation() == HOrientation::Left);
    bool titleTop  = (vOrient == VOrientation::Top);

    float titleY = titleTop ? py : (py + ph - titleH);

    // === Elbow ===
    float ebX = viewLeft ? px : (px + pw - viewW - elbowR);
    float ebY = titleTop ? py : (py + ph - titleH - elbowR);
    dl->AddRectFilled(ImVec2(ebX, ebY), ImVec2(ebX + viewW + elbowR, ebY + titleH + elbowR), kOrange);

    // Arc cutout — center at the inner corner of the elbow block
    float arcCX, arcCY, arcStart;
    if (viewLeft && titleTop)       { arcCX = ebX + viewW + elbowR; arcCY = ebY + titleH + elbowR; arcStart = (float)M_PI; }
    else if (viewLeft && !titleTop) { arcCX = ebX + viewW + elbowR; arcCY = ebY;                   arcStart = (float)M_PI * 0.5f; }
    else if (!viewLeft && titleTop) { arcCX = ebX;                  arcCY = ebY + titleH + elbowR; arcStart = (float)M_PI * 1.5f; }
    else                            { arcCX = ebX;                  arcCY = ebY;                   arcStart = 0.0f; }
    DrawElbowArc(dl, arcCX, arcCY, elbowR, arcStart);

    // === Title bar ===
    float tbX, tbW;
    if (viewLeft) { tbX = px + viewW + elbowR; tbW = pw - viewW - elbowR; }
    else          { tbX = px;                   tbW = pw - viewW - elbowR; }
    dl->AddRectFilled(ImVec2(tbX, titleY), ImVec2(tbX + tbW, titleY + titleH), kOrange);
    // Rounded cap on far end
    if (viewLeft)
        dl->AddRectFilled(ImVec2(tbX + tbW - 20, titleY), ImVec2(tbX + tbW, titleY + titleH), kOrange, 20.0f, ImDrawFlags_RoundCornersRight);
    else
        dl->AddRectFilled(ImVec2(tbX, titleY), ImVec2(tbX + 20, titleY + titleH), kOrange, 20.0f, ImDrawFlags_RoundCornersLeft);

    // Title text (opposite side from elbow)
    const char* title = panel.GetTitle();
    ImVec2 ts = ImGui::CalcTextSize(title);
    float ttX = viewLeft ? (tbX + tbW - ts.x - 20) : (tbX + 20);
    dl->AddText(ImVec2(ttX, titleY + (titleH - ts.y) * 0.5f), IM_COL32(0, 0, 0, 255), title);

    // Elbow text (active view name)
    const auto& views = panel.GetViews();
    int av = panel.activeView;
    if (av < 0 || av >= (int)views.size()) av = 0;
    if (!views.empty()) {
        const char* vn = views[av].name;
        ImVec2 vs = ImGui::CalcTextSize(vn);
        float etX = viewLeft ? (px + (viewW - vs.x) * 0.5f) : (px + pw - viewW + (viewW - vs.x) * 0.5f);
        dl->AddText(ImVec2(etX, titleY + (titleH - vs.y) * 0.5f), IM_COL32(0, 0, 0, 255), vn);
    }

    // === View buttons ===
    float viewX = viewLeft ? px : (px + pw - viewW);
    if ((int)views.size() > 1) {
        float vcTop = titleTop ? (py + titleH + elbowR + gap) : (py + gap);
        float vcBot = titleTop ? (py + ph - gap)              : (py + ph - titleH - elbowR - gap);
        int n = (int)views.size();
        float bh = (vcBot - vcTop - btnGap * (n - 1)) / (float)n;
        if (bh < 20.0f) bh = 20.0f;

        ImGuiIO& io = ImGui::GetIO();
        for (int i = 0; i < n; i++) {
            float by = vcTop + (float)i * (bh + btnGap);
            dl->AddRectFilled(ImVec2(viewX, by), ImVec2(viewX + viewW, by + bh), views[i].buttonColor);
            ImVec2 ls = ImGui::CalcTextSize(views[i].name);
            dl->AddText(ImVec2(viewX + (viewW - ls.x) * 0.5f, by + (bh - ls.y) * 0.5f), IM_COL32(0, 0, 0, 255), views[i].name);
            if (io.MouseClicked[0] && io.MousePos.x >= viewX && io.MousePos.x <= viewX + viewW &&
                io.MousePos.y >= by && io.MousePos.y <= by + bh)
                panel.activeView = i;
        }
    }

    // === Option buttons ===
    float optX = viewLeft ? (px + pw - optW) : px;
    if (!views.empty() && !views[av].optionGroups.empty()) {
        const auto& groups = views[av].optionGroups;
        float ocTop = titleTop ? (py + titleH + gap) : (py + gap);
        float ocBot = titleTop ? (py + ph - gap)     : (py + ph - titleH - gap);

        int total = 0;
        for (const auto& g : groups) total += (int)g.buttons.size();
        int nSep = (int)groups.size() - 1;
        float sepH = 2.0f;
        float bh = (ocBot - ocTop - btnGap * (total - 1) - nSep * (sepH + gap)) / (float)total;
        if (bh < 20.0f) bh = 20.0f;
        float br = bh * 0.5f;

        ImGuiIO& io = ImGui::GetIO();
        float cy = ocTop;
        for (size_t g = 0; g < groups.size(); g++) {
            if (g > 0) {
                cy += gap * 0.5f;
                dl->AddRectFilled(ImVec2(optX + 10, cy), ImVec2(optX + optW - 10, cy + sepH), IM_COL32(80, 80, 80, 255));
                cy += sepH + gap * 0.5f;
            }
            for (const auto& btn : groups[g].buttons) {
                dl->AddRectFilled(ImVec2(optX, cy), ImVec2(optX + optW, cy + bh), btn.color, br, ImDrawFlags_RoundCornersAll);
                ImVec2 ls = ImGui::CalcTextSize(btn.label);
                dl->AddText(ImVec2(optX + (optW - ls.x) * 0.5f, cy + (bh - ls.y) * 0.5f), IM_COL32(0, 0, 0, 255), btn.label);
                if (btn.onClick && io.MouseClicked[0] && io.MousePos.x >= optX && io.MousePos.x <= optX + optW &&
                    io.MousePos.y >= cy && io.MousePos.y <= cy + bh)
                    btn.onClick();
                cy += bh + btnGap;
            }
        }
    }

    // === Content rect ===
    float cx, cw;
    if (viewLeft) { cx = px + viewW + elbowR + gap; cw = pw - viewW - elbowR - optW - gap * 2; }
    else          { cx = px + optW + gap;            cw = pw - optW - viewW - elbowR - gap * 2; }
    float ctop = titleTop ? (py + titleH + gap) : py;
    float ch   = titleTop ? (ph - titleH - gap) : (ph - titleH - gap);

    return ImVec4(cx, ctop, cw, ch);
}
