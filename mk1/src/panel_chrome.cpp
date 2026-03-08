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
    float px, float py, float pw, float ph,
    HOrientation hOrient,
    VOrientation vOrient,
    const std::vector<ViewButtonDef>& views,
    int& activeView,
    const std::vector<OptionGroupDef>& optionGroups
) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    const float titleH = 30.0f;
    const float viewW  = 120.0f;
    const float optW   = 120.0f;
    const float elbowR = 40.0f;
    const float gap    = 6.0f;
    const float btnGap = 4.0f;

    bool viewLeft  = (hOrient == HOrientation::Left);
    bool titleTop  = (vOrient == VOrientation::Top);

    float titleY = titleTop ? py : (py + ph - titleH);

    // Clamp activeView
    if (activeView < 0 || activeView >= (int)views.size()) activeView = 0;

    // Active view name for elbow text
    const char* viewName = views.empty() ? nullptr : views[activeView].name;

    // === Elbow ===
    float ebX = viewLeft ? px : (px + pw - viewW - elbowR);
    float ebY = titleTop ? py : (py + ph - titleH - elbowR);

    if (viewLeft && titleTop) {
        dl->PathLineTo(ImVec2(ebX, ebY - 1));
        dl->PathLineTo(ImVec2(ebX + viewW + elbowR, ebY - 1));
        dl->PathLineTo(ImVec2(ebX + viewW + elbowR, ebY + titleH));
        dl->PathArcTo(ImVec2(ebX + viewW + elbowR, ebY + titleH + elbowR),
                      elbowR, (float)M_PI * 1.5f, (float)M_PI, 32);
        dl->PathLineTo(ImVec2(ebX, ebY + titleH + elbowR));
        dl->PathFillConvex(kOrange);
    } else if (viewLeft && !titleTop) {
        // Bottom-left elbow: L-shape with arc cutout at inner corner
        dl->PathLineTo(ImVec2(ebX, ebY + elbowR));
        dl->PathLineTo(ImVec2(ebX + viewW, ebY + elbowR));
        dl->PathArcTo(ImVec2(ebX + viewW, ebY),
                      elbowR, (float)M_PI * 0.5f, 0.0f, 32);
        dl->PathLineTo(ImVec2(ebX + viewW + elbowR, ebY));
        dl->PathLineTo(ImVec2(ebX, ebY));
        dl->PathFillConvex(kOrange);
    } else if (!viewLeft && titleTop) {
        dl->PathLineTo(ImVec2(ebX, ebY - 1));
        dl->PathLineTo(ImVec2(ebX + elbowR + viewW, ebY - 1));
        dl->PathLineTo(ImVec2(ebX + elbowR + viewW, ebY + titleH + elbowR));
        dl->PathLineTo(ImVec2(ebX + elbowR, ebY + titleH + elbowR));
        dl->PathArcTo(ImVec2(ebX + elbowR, ebY + titleH + elbowR),
                      elbowR, (float)M_PI, (float)M_PI * 0.5f, 32);
        dl->PathFillConvex(kOrange);
    } else {
        // Bottom-right elbow: L-shape with arc cutout at inner corner
        dl->PathLineTo(ImVec2(ebX + viewW + elbowR, ebY + elbowR));
        dl->PathLineTo(ImVec2(ebX + viewW, ebY + elbowR));
        dl->PathArcTo(ImVec2(ebX + viewW, ebY),
                      elbowR, (float)M_PI * 0.5f, (float)M_PI, 32);
        dl->PathLineTo(ImVec2(ebX, ebY));
        dl->PathLineTo(ImVec2(ebX + viewW + elbowR, ebY));
        dl->PathFillConvex(kOrange);
    }

    // === Title bar ===
    float tbX, tbW;
    if (viewLeft) {
        tbX = ebX + viewW + elbowR - 1;
        tbW = pw - viewW - elbowR + 1;
    } else {
        tbX = px;
        tbW = pw - viewW - elbowR + 1;
    }
    dl->AddRectFilled(ImVec2(tbX, titleY), ImVec2(tbX + tbW, titleY + titleH), kOrange);
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
            dl->AddRectFilled(ImVec2(viewX, by), ImVec2(viewX + viewW, by + bh), views[i].color);
            ImVec2 ls = ImGui::CalcTextSize(views[i].name);
            dl->AddText(ImVec2(viewX + (viewW - ls.x) * 0.5f, by + (bh - ls.y) * 0.5f), IM_COL32(0, 0, 0, 255), views[i].name);
            if (io.MouseClicked[0] && io.MousePos.x >= viewX && io.MousePos.x <= viewX + viewW &&
                io.MousePos.y >= by && io.MousePos.y <= by + bh)
                activeView = i;
        }
    }

    // === Option buttons ===
    float optX = viewLeft ? (px + pw - optW) : px;
    if (!optionGroups.empty()) {
        float ocTop = titleTop ? (py + titleH + gap) : (py + gap);
        float ocBot = titleTop ? (py + ph - gap)     : (py + ph - titleH - gap);

        int total = 0;
        for (const auto& g : optionGroups) total += (int)g.buttons.size();
        int nSep = (int)optionGroups.size() - 1;
        float sepH = 2.0f;
        float bh = (ocBot - ocTop - btnGap * (total - 1) - nSep * (sepH + gap)) / (float)total;
        if (bh < 20.0f) bh = 20.0f;
        float br = bh * 0.5f;

        float cy = ocTop;
        for (size_t g = 0; g < optionGroups.size(); g++) {
            if (g > 0) {
                cy += gap * 0.5f;
                dl->AddRectFilled(ImVec2(optX + 10, cy), ImVec2(optX + optW - 10, cy + sepH), IM_COL32(80, 80, 80, 255));
                cy += sepH + gap * 0.5f;
            }
            for (const auto& btn : optionGroups[g].buttons) {
                dl->AddRectFilled(ImVec2(optX, cy), ImVec2(optX + optW, cy + bh), btn.color, br, ImDrawFlags_RoundCornersAll);
                ImVec2 ls = ImGui::CalcTextSize(btn.label);
                dl->AddText(ImVec2(optX + (optW - ls.x) * 0.5f, cy + (bh - ls.y) * 0.5f), IM_COL32(0, 0, 0, 255), btn.label);
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
