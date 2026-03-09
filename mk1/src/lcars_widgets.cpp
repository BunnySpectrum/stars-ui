#include "lcars_widgets.h"
#include "theme.h"

// LCARS elbow geometry constants
static const float kViewW = 120.0f;
static const float kElbowR = 40.0f;
static const float kTitleH = 30.0f;

void DrawElbowOverdraw(
    ImDrawList* dl,
    float x, float y,
    ElbowCorner corner,
    ImU32 color,
    bool drawDebugVertices)
{
    float ebX = x;
    float ebY = y;
    float arcCX, arcCY;
    ImVec2 rectMin, rectMax;
    ImVec2 tangent1, tangent2;

    // Calculate geometry based on corner orientation
    switch (corner)
    {
    case ElbowCorner::TopLeft:
        rectMin = ImVec2(ebX, ebY - 1);
        rectMax = ImVec2(ebX + kViewW + kElbowR, ebY + kTitleH + kElbowR);
        arcCX = ebX + kViewW + kElbowR;
        arcCY = ebY + kTitleH + kElbowR;
        tangent1 = ImVec2(ebX + kViewW, arcCY);
        tangent2 = ImVec2(arcCX, ebY + kTitleH);
        break;

    case ElbowCorner::TopRight:
        rectMin = ImVec2(ebX - kViewW - kElbowR, ebY - 1);
        rectMax = ImVec2(ebX, ebY + kTitleH + kElbowR);
        arcCX = ebX - kViewW - kElbowR;
        arcCY = ebY + kTitleH + kElbowR;
        tangent1 = ImVec2(ebX - kViewW, arcCY);
        tangent2 = ImVec2(arcCX, ebY + kTitleH);
        break;

    case ElbowCorner::BottomLeft:
        rectMin = ImVec2(ebX, ebY);
        rectMax = ImVec2(ebX + kViewW + kElbowR, ebY + kTitleH + kElbowR);
        arcCX = ebX + kViewW + kElbowR;
        arcCY = ebY;
        tangent1 = ImVec2(ebX + kViewW, arcCY);
        tangent2 = ImVec2(arcCX, ebY + kElbowR);
        break;

    case ElbowCorner::BottomRight:
        rectMin = ImVec2(ebX, ebY);
        rectMax = ImVec2(ebX + kElbowR + kViewW, ebY + kTitleH + kElbowR);
        arcCX = ebX;
        arcCY = ebY;
        tangent1 = ImVec2(ebX + kElbowR, arcCY);
        tangent2 = ImVec2(arcCX, ebY + kElbowR);
        break;
    }

    // Draw filled rectangle (entire elbow area)
    dl->AddRectFilled(rectMin, rectMax, color);

    // Cut out inner corner with black circle
    dl->AddCircleFilled(ImVec2(arcCX, arcCY), kElbowR, IM_COL32(0, 0, 0, 255), 32);

    // Optional debug visualization
    if (drawDebugVertices)
    {
        const float dotRadius = 3.0f;
        const ImU32 dotColor = IM_COL32(255, 255, 0, 255); // Yellow

        // Rectangle vertices
        dl->AddCircleFilled(ImVec2(rectMin.x, rectMin.y), dotRadius, dotColor);
        dl->AddCircleFilled(ImVec2(rectMax.x, rectMin.y), dotRadius, dotColor);
        dl->AddCircleFilled(ImVec2(rectMax.x, rectMax.y), dotRadius, dotColor);
        dl->AddCircleFilled(ImVec2(rectMin.x, rectMax.y), dotRadius, dotColor);

        // Arc tangent points
        dl->AddCircleFilled(tangent1, dotRadius, dotColor);
        dl->AddCircleFilled(tangent2, dotRadius, dotColor);

        // Circle center
        dl->AddCircleFilled(ImVec2(arcCX, arcCY), dotRadius, IM_COL32(255, 0, 0, 255));
    }
}
