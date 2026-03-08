#include "panel_chrome.h"
#include "theme.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ---------------------------------------------------------------------------
// Global chrome
// ---------------------------------------------------------------------------

void DrawGlobalTopBar(float W) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const float h = 40.0f;
    dl->AddRectFilled(ImVec2(0, 0), ImVec2(W, h), kOrange);
    dl->AddRectFilled(ImVec2(W - 20, 0), ImVec2(W, h), kOrange, 20.0f, ImDrawFlags_RoundCornersRight);

    // Draw "STARS" in Aurebesh on the title bar
    if (ImFont* font = GetAurebeshFont()) {
        const char* aurebeshText = "STARS";
        float fontSize = 24.0f;
        ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, aurebeshText);
        float textX = 20.0f;
        float textY = (h - textSize.y) * 0.5f;
        dl->AddText(font, fontSize, ImVec2(textX, textY), IM_COL32(0, 0, 0, 255), aurebeshText);
    }
}

void DrawGlobalBottomBar(float W, float H) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const float h = 30.0f;
    const float inset = 6.0f;
    dl->AddRectFilled(ImVec2(inset, H - h), ImVec2(W, H), kTan);
    dl->AddRectFilled(ImVec2(inset, H - h), ImVec2(inset + 20, H), kTan, 20.0f, ImDrawFlags_RoundCornersLeft);
}

// ---------------------------------------------------------------------------
// Per-panel chrome
// ---------------------------------------------------------------------------

ImVec4 DrawPanelChrome(
    const char* title,
    const char* viewName,
    float px, float py, float pw, float ph,
    HOrientation hOrient,
    VOrientation vOrient
) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    const float titleH = 30.0f;
    const float viewW  = 120.0f;
    const float optW   = 120.0f;
    const float elbowR = 40.0f;
    const float gap    = 6.0f;

    bool viewLeft  = (hOrient == HOrientation::Left);
    bool titleTop  = (vOrient == VOrientation::Top);

    float titleY = titleTop ? py : (py + ph - titleH);

    // === Elbow ===
    // Draw elbow as a single filled path with smooth arc
    float ebX = viewLeft ? px : (px + pw - viewW - elbowR);
    float ebY = titleTop ? py : (py + ph - titleH - elbowR);

    // Build the elbow path (L-shape with rounded inner corner)
    // Overlap by 1px with global bars to avoid gaps
    if (viewLeft && titleTop) {
        // Top-left elbow: trace clockwise from top-left, extend top edge up 1px
        dl->PathLineTo(ImVec2(ebX, ebY - 1));                                // Top-left corner (overlap with global bar)
        dl->PathLineTo(ImVec2(ebX + viewW + elbowR, ebY - 1));              // Top-right corner (overlap with global bar)
        dl->PathLineTo(ImVec2(ebX + viewW + elbowR, ebY + titleH));         // Right edge
        dl->PathArcTo(ImVec2(ebX + viewW + elbowR, ebY + titleH + elbowR),  // Arc center
                      elbowR, (float)M_PI * 1.5f, (float)M_PI, 32);          // 270° to 180° (32 segments)
        dl->PathLineTo(ImVec2(ebX, ebY + titleH + elbowR));                 // Bottom-left corner
        dl->PathFillConvex(kOrange);
    } else if (viewLeft && !titleTop) {
        // Bottom-left elbow: trace clockwise from bottom-left
        dl->PathLineTo(ImVec2(ebX, ebY + titleH + elbowR));
        dl->PathLineTo(ImVec2(ebX, ebY));
        dl->PathLineTo(ImVec2(ebX + viewW + elbowR, ebY));
        dl->PathLineTo(ImVec2(ebX + viewW + elbowR, ebY + elbowR));
        dl->PathArcTo(ImVec2(ebX + viewW + elbowR, ebY + elbowR),
                      elbowR, 0.0f, (float)M_PI * 0.5f, 32);                 // 0° to 90° (32 segments)
        dl->PathFillConvex(kOrange);
    } else if (!viewLeft && titleTop) {
        // Top-right elbow: trace clockwise from top-left, extend top edge up 1px
        dl->PathLineTo(ImVec2(ebX, ebY - 1));
        dl->PathLineTo(ImVec2(ebX + elbowR + viewW, ebY - 1));
        dl->PathLineTo(ImVec2(ebX + elbowR + viewW, ebY + titleH + elbowR));
        dl->PathLineTo(ImVec2(ebX + elbowR, ebY + titleH + elbowR));
        dl->PathArcTo(ImVec2(ebX + elbowR, ebY + titleH + elbowR),
                      elbowR, (float)M_PI, (float)M_PI * 0.5f, 32);          // 180° to 90° (32 segments)
        dl->PathFillConvex(kOrange);
    } else { // !viewLeft && !titleTop
        // Bottom-right elbow: trace clockwise
        dl->PathLineTo(ImVec2(ebX + elbowR, ebY));
        dl->PathLineTo(ImVec2(ebX + elbowR + viewW, ebY));
        dl->PathLineTo(ImVec2(ebX + elbowR + viewW, ebY + titleH + elbowR));
        dl->PathLineTo(ImVec2(ebX + elbowR, ebY + elbowR));
        dl->PathArcTo(ImVec2(ebX + elbowR, ebY + elbowR),
                      elbowR, (float)M_PI * 0.5f, 0.0f, 32);                 // 90° to 0° (32 segments)
        dl->PathFillConvex(kOrange);
    }

    // === Title bar ===
    // Overlap slightly with elbow to avoid 1px gaps from anti-aliasing
    float tbX, tbW;
    if (viewLeft) {
        tbX = ebX + viewW + elbowR - 1;  // Overlap 1px with elbow
        tbW = pw - viewW - elbowR + 1;
    }
    else {
        tbX = px;
        tbW = pw - viewW - elbowR + 1;   // Overlap 1px with elbow
    }
    dl->AddRectFilled(ImVec2(tbX, titleY), ImVec2(tbX + tbW, titleY + titleH), kOrange);
    // Rounded cap on far end
    if (viewLeft)
        dl->AddRectFilled(ImVec2(tbX + tbW - 20, titleY), ImVec2(tbX + tbW, titleY + titleH), kOrange, 20.0f, ImDrawFlags_RoundCornersRight);
    else
        dl->AddRectFilled(ImVec2(tbX, titleY), ImVec2(tbX + 20, titleY + titleH), kOrange, 20.0f, ImDrawFlags_RoundCornersLeft);

    // Title text (opposite side from elbow)
    ImVec2 ts = ImGui::CalcTextSize(title);
    float ttX = viewLeft ? (tbX + tbW - ts.x - 20) : (tbX + 20);
    dl->AddText(ImVec2(ttX, titleY + (titleH - ts.y) * 0.5f), IM_COL32(0, 0, 0, 255), title);

    // Elbow text (active view name)
    if (viewName) {
        ImVec2 vs = ImGui::CalcTextSize(viewName);
        float etX = viewLeft ? (px + (viewW - vs.x) * 0.5f) : (px + pw - viewW + (viewW - vs.x) * 0.5f);
        dl->AddText(ImVec2(etX, titleY + (titleH - vs.y) * 0.5f), IM_COL32(0, 0, 0, 255), viewName);
    }

    // === Content rect ===
    float cx, cw;
    if (viewLeft) { cx = px + viewW + elbowR + gap; cw = pw - viewW - elbowR - optW - gap * 2; }
    else          { cx = px + optW + gap;            cw = pw - optW - viewW - elbowR - gap * 2; }
    float ctop = titleTop ? (py + titleH + gap) : py;
    float ch   = titleTop ? (ph - titleH - gap) : (ph - titleH - gap);

    return ImVec4(cx, ctop, cw, ch);
}
