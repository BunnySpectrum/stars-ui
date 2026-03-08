#include <GLFW/glfw3.h>
#include <cstdio>
#include <cmath>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include "theme.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// LCARS elbow constants
static const float kViewW = 120.0f;
static const float kElbowR = 40.0f;
static const float kTitleH = 30.0f;
static const float kSweep = 1 * (float)M_PI * 0.5f;

// Colors for comparison
static const ImU32 kColorPath = kOrange;     // Orange for path method
static const ImU32 kColorOverdraw = kPurple; // Purple for overdraw method
static const ImU32 kColorBlack = IM_COL32(0, 0, 0, 255);

static void glfwErrorCallback(int error, const char *description)
{
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

void DrawElbowTopLeft(ImDrawList *dl, float x, float y)
{
    float ebX = x;
    float ebY = y;
    const float kAngleStart = (float)M_PI * 1.5f;
    const float kAngleEnd = kAngleStart - kSweep;

    dl->PathLineTo(ImVec2(ebX, ebY - 1));
    dl->PathLineTo(ImVec2(ebX + kViewW + kElbowR, ebY - 1));
    dl->PathLineTo(ImVec2(ebX + kViewW + kElbowR, ebY + kTitleH));
    dl->PathArcTo(ImVec2(ebX + kViewW + kElbowR, ebY + kTitleH + kElbowR),
                  kElbowR, kAngleStart, kAngleEnd, 32);
    dl->PathLineTo(ImVec2(ebX, ebY + kTitleH + kElbowR));
    dl->PathFillConcave(kOrange);
}

void DrawElbowTopRightCW(ImDrawList *dl, float x, float y)
{
    float ebX = x;
    float ebY = y;
    const float kAngleStart = (float)M_PI * 0.0f;
    const float kAngleEnd = kAngleStart - kSweep;

    dl->PathLineTo(ImVec2(ebX, ebY - 1));                                    // top-left
    dl->PathLineTo(ImVec2(ebX + kViewW + kElbowR, ebY - 1));                 // top-right
    dl->PathLineTo(ImVec2(ebX + kViewW + kElbowR, ebY + kTitleH + kElbowR)); // bot-right
    dl->PathLineTo(ImVec2(ebX + kElbowR, ebY + kTitleH + kElbowR));          // bot-left, pre-arc

    dl->PathArcTo(ImVec2(ebX, ebY + kTitleH + kElbowR),
                  kElbowR, kAngleStart, kAngleEnd, 32);

    dl->PathFillConvex(kOrange);
}
void DrawElbowTopRight(ImDrawList *dl, float x, float y)
{
    float ebX = x;
    float ebY = y;
    const float kAngleStart = (float)M_PI * 1.5f;
    const float kAngleEnd = kAngleStart + kSweep;

    dl->PathLineTo(ImVec2(ebX, ebY - 1));
    dl->PathLineTo(ImVec2(ebX - kViewW - kElbowR, ebY - 1));
    dl->PathLineTo(ImVec2(ebX - kViewW - kElbowR, ebY + kTitleH));

    dl->PathArcTo(ImVec2(ebX - kViewW - kElbowR, ebY + kTitleH + kElbowR),
                  kElbowR, kAngleStart, kAngleEnd, 64);

    dl->PathLineTo(ImVec2(ebX, ebY + kTitleH + kElbowR));
    dl->PathFillConvex(kOrange);
}

void DrawElbowBottomLeft(ImDrawList *dl, float x, float y)
{
    float ebX = x;
    float ebY = y;
    const float kAngleStart = (float)M_PI * 1.0f;
    const float kAngleEnd = kAngleStart - kSweep;

    // CW
    dl->PathLineTo(ImVec2(ebX, ebY));
    dl->PathLineTo(ImVec2(ebX + kViewW, ebY));

    dl->PathArcTo(ImVec2(ebX + kViewW + kElbowR, ebY),
                  kElbowR, kAngleStart, kAngleEnd, 32);

    dl->PathLineTo(ImVec2(ebX + kViewW + kElbowR, ebY + kTitleH + kElbowR));
    dl->PathLineTo(ImVec2(ebX, ebY + kTitleH + kElbowR));
    dl->PathFillConcave(kOrange);
}

void DrawElbowBottomRight(ImDrawList *dl, float x, float y)
{
    float ebX = x;
    float ebY = y;
    const float kAngleStart = (float)M_PI * 0.0f;
    const float kAngleEnd = kAngleStart + kSweep;

    dl->PathLineTo(ImVec2(ebX + kElbowR, ebY));
    dl->PathArcTo(ImVec2(ebX, ebY),
                  kElbowR, kAngleStart, kAngleEnd, 32);
    dl->PathLineTo(ImVec2(ebX, ebY + kTitleH + kElbowR));
    dl->PathLineTo(ImVec2(ebX + kElbowR + kViewW, ebY + kTitleH + kElbowR));
    dl->PathLineTo(ImVec2(ebX + kElbowR + kViewW, ebY));
    dl->PathFillConcave(kOrange);
}

// ============================================================================
// OVERDRAW METHOD: Rectangle + circle cutout
// ============================================================================

void DrawElbowTopLeftOverdraw(ImDrawList *dl, float x, float y)
{
    float ebX = x;
    float ebY = y;

    // Draw filled rectangle (entire elbow area)
    dl->AddRectFilled(ImVec2(ebX, ebY - 1),
                      ImVec2(ebX + kViewW + kElbowR, ebY + kTitleH + kElbowR),
                      kColorOverdraw);

    // Cut out inner corner with black circle
    float arcCX = ebX + kViewW + kElbowR;
    float arcCY = ebY + kTitleH + kElbowR;
    dl->AddCircleFilled(ImVec2(arcCX, arcCY), kElbowR, kColorBlack, 32);
}

void DrawElbowTopRightOverdraw(ImDrawList *dl, float x, float y)
{
    float ebX = x;
    float ebY = y;

    // Draw filled rectangle (entire elbow area)
    dl->AddRectFilled(ImVec2(ebX - kViewW - kElbowR, ebY - 1),
                      ImVec2(ebX, ebY + kTitleH + kElbowR),
                      kColorOverdraw);

    // Cut out inner corner with black circle
    float arcCX = ebX - kViewW - kElbowR;
    float arcCY = ebY + kTitleH + kElbowR;
    dl->AddCircleFilled(ImVec2(arcCX, arcCY), kElbowR, kColorBlack, 32);
}

void DrawElbowBottomLeftOverdraw(ImDrawList *dl, float x, float y)
{
    float ebX = x;
    float ebY = y;

    // Draw filled rectangle (entire elbow area)
    dl->AddRectFilled(ImVec2(ebX, ebY),
                      ImVec2(ebX + kViewW + kElbowR, ebY + kTitleH + kElbowR),
                      kColorOverdraw);

    // Cut out inner corner with black circle
    float arcCX = ebX + kViewW + kElbowR;
    float arcCY = ebY;
    dl->AddCircleFilled(ImVec2(arcCX, arcCY), kElbowR, kColorBlack, 32);
}

void DrawElbowBottomRightOverdraw(ImDrawList *dl, float x, float y)
{
    float ebX = x;
    float ebY = y;

    // Draw filled rectangle (entire elbow area)
    dl->AddRectFilled(ImVec2(ebX, ebY),
                      ImVec2(ebX + kElbowR + kViewW, ebY + kTitleH + kElbowR),
                      kColorOverdraw);

    // Cut out inner corner with black circle
    float arcCX = ebX;
    float arcCY = ebY;
    dl->AddCircleFilled(ImVec2(arcCX, arcCY), kElbowR, kColorBlack, 32);
}

int main(int argc, char **argv)
{
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit())
    {
        std::fprintf(stderr, "glfwInit failed\n");
        return 1;
    }

#ifdef __APPLE__
    const char *glslVersion = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char *glslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    GLFWwindow *window = glfwCreateWindow(800, 600, "LCARS Elbow Corner Demo", nullptr, nullptr);
    if (!window)
    {
        std::fprintf(stderr, "glfwCreateWindow failed\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);

    io.Fonts->AddFontFromFileTTF("poc/fonts/Antonio-Regular.ttf", 20.0f);

    // Apply basic dark theme (skip full STARS theme to avoid ImPlot dependency)
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowBorderSize = 0.0f;
    ImVec4 *colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.04f, 1.00f);
    colors[ImGuiCol_Text] = U32ToVec4(kOrange);

    std::printf("LCARS Elbow Corner Demo\n");
    std::printf("Drawing all 4 elbow orientations\n");

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int winW, winH;
        glfwGetWindowSize(window, &winW, &winH);

        ImDrawList *dl = ImGui::GetBackgroundDrawList();

        // Grid layout: 2x2
        float cellW = winW / 2.0f;
        float cellH = winH / 2.0f;
        float centerX = cellW / 2.0f;
        float centerY = cellH / 2.0f;

        // Draw grid lines for reference
        dl->AddLine(ImVec2(cellW, 0), ImVec2(cellW, winH), IM_COL32(60, 60, 60, 255), 1.0f);
        dl->AddLine(ImVec2(0, cellH), ImVec2(winW, cellH), IM_COL32(60, 60, 60, 255), 1.0f);

        // Legend
        float legendX = winW - 200;
        float legendY = 10;
        dl->AddRectFilled(ImVec2(legendX, legendY), ImVec2(legendX + 30, legendY + 20), kColorPath);
        dl->AddText(ImVec2(legendX + 35, legendY + 2), IM_COL32(255, 153, 51, 255), "Path Method");

        dl->AddRectFilled(ImVec2(legendX, legendY + 25), ImVec2(legendX + 30, legendY + 45), kColorOverdraw);
        dl->AddText(ImVec2(legendX + 35, legendY + 27), IM_COL32(204, 153, 204, 255), "Overdraw Method");

        // Labels
        dl->AddText(ImVec2(10, 10), IM_COL32(255, 153, 51, 255), "TOP-LEFT");
        dl->AddText(ImVec2(cellW + 10, 10), IM_COL32(255, 153, 51, 255), "TOP-RIGHT");
        dl->AddText(ImVec2(10, cellH + 10), IM_COL32(255, 153, 51, 255), "BOTTOM-LEFT");
        dl->AddText(ImVec2(cellW + 10, cellH + 10), IM_COL32(255, 153, 51, 255), "BOTTOM-RIGHT");

        const float vOffset = 120.0f; // Vertical offset to stack overdraw below path

        // Top-left elbow - both versions
        DrawElbowTopLeft(dl, centerX - 80, centerY - 80);
        DrawElbowTopLeftOverdraw(dl, centerX - 80, centerY - 80 + vOffset);

        // Top-right elbow - both versions
        DrawElbowTopRight(dl, cellW + centerX + 80, centerY - 80);
        DrawElbowTopRightOverdraw(dl, cellW + centerX + 80, centerY - 80 + vOffset);

        // Bottom-left elbow - both versions
        DrawElbowBottomLeft(dl, centerX - 80, cellH + centerY - 40);
        DrawElbowBottomLeftOverdraw(dl, centerX - 80, cellH + centerY - 40 + vOffset);

        // Bottom-right elbow - both versions
        DrawElbowBottomRight(dl, cellW + centerX - 80, cellH + centerY - 40);
        DrawElbowBottomRightOverdraw(dl, cellW + centerX - 80, cellH + centerY - 40 + vOffset);

        // Render
        ImGui::Render();
        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
