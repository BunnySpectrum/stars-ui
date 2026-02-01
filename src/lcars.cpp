#include "lcars.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// LCARS color palette
const ImU32 kOrange  = IM_COL32(0xFF, 0x99, 0x33, 0xFF);
const ImU32 kPurple  = IM_COL32(0xCC, 0x99, 0xCC, 0xFF);
const ImU32 kBlue    = IM_COL32(0x99, 0x99, 0xFF, 0xFF);
const ImU32 kTan     = IM_COL32(0xFF, 0xCC, 0x99, 0xFF);
const ImU32 kRed     = IM_COL32(0xCC, 0x66, 0x66, 0xFF);
const ImU32 kBeige   = IM_COL32(0xFF, 0xDD, 0xBB, 0xFF);

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

    // ImPlot styling
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

void DrawLCARSFrame(float W, float H, float dividerY) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    // Dimensions
    const float topBarH    = 40.0f;
    const float sidebarW   = 120.0f;
    const float elbowH     = 80.0f;
    const float bottomBarH = 30.0f;
    const float divBarH    = 30.0f;
    const float divElbowR  = 40.0f;  // radius of the small divider elbow curve
    const float gap        = 6.0f;

    // === Top bar (right of elbow) ===
    dl->AddRectFilled(
        ImVec2(sidebarW + elbowH, 0),
        ImVec2(W, topBarH),
        kOrange, 0.0f, 0
    );
    dl->AddRectFilled(
        ImVec2(W - 20, 0),
        ImVec2(W, topBarH),
        kOrange, 20.0f, ImDrawFlags_RoundCornersRight
    );

    // === Top-left elbow ===
    dl->AddRectFilled(
        ImVec2(0, 0),
        ImVec2(sidebarW + elbowH, topBarH + elbowH),
        kOrange, 0.0f, 0
    );
    // Cut out the inner arc
    {
        const ImVec2 arcCenter(sidebarW + elbowH, topBarH + elbowH);
        const int arcSegments = 32;
        ImVector<ImVec2> arcPoints;
        for (int i = 0; i <= arcSegments; i++) {
            float angle = (float)M_PI + ((float)M_PI * 0.5f) * ((float)i / (float)arcSegments);
            arcPoints.push_back(ImVec2(
                arcCenter.x + cosf(angle) * elbowH,
                arcCenter.y + sinf(angle) * elbowH
            ));
        }
        arcPoints.push_back(arcCenter);
        dl->AddConvexPolyFilled(arcPoints.Data, arcPoints.Size, IM_COL32(0, 0, 0, 255));
    }

    // Button in the top-left elbow area
    {
        const float btnW = sidebarW - 10.0f;
        const float btnH = 28.0f;
        const float btnX = 5.0f;
        const float btnY = topBarH + 6.0f;
        dl->AddRectFilled(
            ImVec2(btnX, btnY),
            ImVec2(btnX + btnW, btnY + btnH),
            kPurple, 14.0f, ImDrawFlags_RoundCornersAll
        );
        const char* btnLabel = "HOME";
        ImVec2 btnTextSize = ImGui::CalcTextSize(btnLabel);
        dl->AddText(
            ImVec2(btnX + (btnW - btnTextSize.x) * 0.5f, btnY + (btnH - btnTextSize.y) * 0.5f),
            IM_COL32(0, 0, 0, 255),
            btnLabel
        );
    }

    // === Sidebar blocks above divider (1 block) ===
    {
        const float upperSideTop = topBarH + elbowH + gap;
        const float upperSideBot = dividerY - divElbowR - gap;
        float blockH = upperSideBot - upperSideTop;
        if (blockH > 0) {
            dl->AddRectFilled(
                ImVec2(0, upperSideTop), ImVec2(sidebarW, upperSideBot),
                kPurple, 20.0f, ImDrawFlags_RoundCornersLeft
            );
        }
    }

    // === Divider bar with small elbow ===
    // Small elbow: filled block connecting sidebar to divider bar
    dl->AddRectFilled(
        ImVec2(0, dividerY - divElbowR),
        ImVec2(sidebarW + divElbowR, dividerY + divBarH),
        kOrange, 0.0f, 0
    );
    // Cut out inner arc of the small elbow (quarter-circle in top-right of the elbow block)
    {
        const ImVec2 arcCenter(sidebarW + divElbowR, dividerY - divElbowR);
        const int arcSegments = 32;
        ImVector<ImVec2> arcPoints;
        for (int i = 0; i <= arcSegments; i++) {
            float angle = 0.0f + ((float)M_PI * 0.5f) * ((float)i / (float)arcSegments);
            arcPoints.push_back(ImVec2(
                arcCenter.x + cosf(angle) * divElbowR,
                arcCenter.y + sinf(angle) * divElbowR
            ));
        }
        arcPoints.push_back(arcCenter);
        dl->AddConvexPolyFilled(arcPoints.Data, arcPoints.Size, IM_COL32(0, 0, 0, 255));
    }
    // Divider bar extending to the right
    dl->AddRectFilled(
        ImVec2(sidebarW + divElbowR, dividerY),
        ImVec2(W, dividerY + divBarH),
        kOrange, 0.0f, 0
    );
    dl->AddRectFilled(
        ImVec2(W - 20, dividerY),
        ImVec2(W, dividerY + divBarH),
        kOrange, 20.0f, ImDrawFlags_RoundCornersRight
    );

    // === Sidebar blocks below divider (3 blocks) ===
    {
        const float lowerSideTop = dividerY + divBarH + gap;
        const float lowerSideBot = H - bottomBarH - gap;
        const float blockH = (lowerSideBot - lowerSideTop - gap * 2) / 3.0f;
        const ImU32 lowerColors[] = { kBlue, kTan, kOrange };
        for (int i = 0; i < 3; i++) {
            float y0 = lowerSideTop + i * (blockH + gap);
            float y1 = y0 + blockH;
            dl->AddRectFilled(
                ImVec2(0, y0), ImVec2(sidebarW, y1),
                lowerColors[i], 20.0f, ImDrawFlags_RoundCornersLeft
            );
        }
    }

    // === Bottom bar ===
    dl->AddRectFilled(
        ImVec2(sidebarW + gap, H - bottomBarH),
        ImVec2(W, H),
        kTan, 0.0f, 0
    );
    dl->AddRectFilled(
        ImVec2(sidebarW + gap, H - bottomBarH),
        ImVec2(sidebarW + gap + 20, H),
        kTan, 20.0f, ImDrawFlags_RoundCornersLeft
    );
}
