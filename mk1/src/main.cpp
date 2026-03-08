#include <GLFW/glfw3.h>
#include <cstdio>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "implot/implot.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include "theme.h"
#include "panel_chrome.h"
#include "demo_widgets.h"

static void glfwErrorCallback(int error, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

// Panel state persisted across frames
static int g_panel1ActiveView = 0;
static int g_panel2ActiveView = 0;

int main(int argc, char** argv) {
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit()) {
        std::fprintf(stderr, "glfwInit failed\n");
        return 1;
    }

    // OpenGL attributes
#ifdef __APPLE__
    const char* glslVersion = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    GLFWwindow* window = glfwCreateWindow(1280, 800, "MK1 - STARS LCARS Demo", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "glfwCreateWindow failed\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    // ImGui + ImPlot setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // don't save imgui.ini

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);

    // Load fonts
    io.Fonts->AddFontFromFileTTF("poc/fonts/Antonio-Regular.ttf", 20.0f);
    ImFont* aurebeshFont = io.Fonts->AddFontFromFileTTF("poc/fonts/AurebeshAF-Canon.ttf", 24.0f);
    SetAurebeshFont(aurebeshFont);

    ApplySTARSTheme();

    std::printf("MK1: STARS LCARS Feature Demo\n");

    // ---------------------------------------------------------------------------
    // Panel 1: SYSTEM STATUS - view definitions
    // ---------------------------------------------------------------------------
    std::vector<ViewButtonDef> panel1Views = {
        {"OVR", kPurple},
        {"GRF", kBlue},
        {"DTL", kTan},
    };
    std::vector<OptionGroupDef> panel1Opts = {
        {{{ "UNITS", kPurple }, { "SCALE", kBlue }}},
        {{{ "RESET", kRed }}},
    };

    // ---------------------------------------------------------------------------
    // Panel 2: OPERATIONS - view definitions
    // ---------------------------------------------------------------------------
    std::vector<ViewButtonDef> panel2Views = {
        {"STS", kPurple},
        {"LOG", kBlue},
    };
    std::vector<OptionGroupDef> panel2Opts = {
        {{{ "AUTO", kPurple }, { "MANUAL", kTan }}},
    };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int winW, winH;
        glfwGetWindowSize(window, &winW, &winH);
        float W = (float)winW;
        float H = (float)winH;

        // Global chrome
        DrawGlobalTopBar(W);
        DrawGlobalBottomBar(W, H);

        // Panel layout constants
        const float topBarH    = 40.0f;
        const float bottomBarH = 30.0f;
        const float panelGap   = 10.0f;

        float panelTop = topBarH;
        float panelAreaH = H - topBarH - bottomBarH;

        // Two panels: 60/40 split
        float panel1H = panelAreaH * 0.6f - panelGap * 0.5f;
        float panel2H = panelAreaH * 0.4f - panelGap * 0.5f;

        float panel1Y = panelTop;
        float panel2Y = panelTop + panel1H + panelGap;

        // =====================================================================
        // Panel 1: SYSTEM STATUS (Bottom orientation = title at bottom)
        // =====================================================================
        ImVec4 cr1 = DrawPanelChrome(
            "SYSTEM STATUS",
            0.0f, panel1Y, W, panel1H,
            HOrientation::Left, VOrientation::Bottom,
            panel1Views, g_panel1ActiveView, panel1Opts
        );

        {
            ImGui::SetNextWindowPos(ImVec2(cr1.x, cr1.y));
            ImGui::SetNextWindowSize(ImVec2(cr1.z, cr1.w));
            ImGui::Begin("##Panel1Content", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);

            if (g_panel1ActiveView == 0) {
                // OVR: Overview - section headers, label/values, progress bars
                DrawSectionHeader("PROPULSION");
                DrawLabelValue("WARP FACTOR", "6.2");
                DrawLabelValue("IMPULSE",     "FULL STOP");
                DrawLabelValue("HEADING",     "127 MARK 4");

                DrawSectionHeader("POWER DISTRIBUTION");
                DrawProgressBar("WARP CORE",    0.92f, 0.80f, 0.95f);
                DrawProgressBar("SHIELDS",      0.45f, 0.70f, 0.90f);
                DrawProgressBar("LIFE SUPPORT", 1.00f, 0.80f, 0.95f);
                DrawProgressBar("WEAPONS",      0.00f, 0.70f, 0.90f);
                DrawProgressBar("SENSORS",      0.87f, 0.80f, 0.95f);

                DrawSectionHeader("ENVIRONMENTAL");
                DrawLabelValue("TEMPERATURE", "22.1 C");
                DrawLabelValue("PRESSURE",    "101.3 kPa");
                DrawLabelValue("OXYGEN",      "20.9%");

            } else if (g_panel1ActiveView == 1) {
                // GRF: Graph view - animated ImPlot chart
                DrawSectionHeader("WARP FIELD ANALYSIS");
                DrawDemoChart();

            } else if (g_panel1ActiveView == 2) {
                // DTL: Detail view - table + color palette
                DrawSectionHeader("SUBSYSTEM STATUS");
                DrawDemoTable();

                DrawSectionHeader("LCARS COLOR PALETTE");
                DrawColorPalette();
            }

            ImGui::End();
        }

        // =====================================================================
        // Panel 2: OPERATIONS (Top orientation = title at top)
        // =====================================================================
        ImVec4 cr2 = DrawPanelChrome(
            "OPERATIONS",
            0.0f, panel2Y, W, panel2H,
            HOrientation::Left, VOrientation::Top,
            panel2Views, g_panel2ActiveView, panel2Opts
        );

        {
            ImGui::SetNextWindowPos(ImVec2(cr2.x, cr2.y));
            ImGui::SetNextWindowSize(ImVec2(cr2.z, cr2.w));
            ImGui::Begin("##Panel2Content", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);

            if (g_panel2ActiveView == 0) {
                // STS: Status - threshold coloring showcase
                DrawSectionHeader("ALERT STATUS");
                DrawLabelValueThreshold("HULL INTEGRITY", "98.2%",  0.982f, 0.50f, 0.25f);
                DrawLabelValueThreshold("SHIELD POWER",   "45.0%",  0.450f, 0.50f, 0.25f);
                DrawLabelValueThreshold("ANTIMATTER",     "12.1%",  0.121f, 0.50f, 0.25f);

                DrawSectionHeader("CREW STATUS");
                DrawLabelValue("COMPLEMENT",   "1,012 / 1,014");
                DrawLabelValue("CASUALTIES",   "0");
                DrawLabelValue("AWAY TEAMS",   "2 DEPLOYED");

                DrawSectionHeader("TACTICAL");
                DrawLabelValueThreshold("PHASER BANKS",    "OFFLINE", 0.0f, 0.01f, 0.001f);
                DrawLabelValueThreshold("TORPEDO TUBES",   "4 / 4",   1.0f, 0.50f, 0.25f);
                DrawLabelValueThreshold("TRACTOR BEAM",    "STANDBY", 0.5f, 0.70f, 0.90f);

            } else if (g_panel2ActiveView == 1) {
                // LOG: Scrolling operations log
                DrawSectionHeader("OPERATIONS LOG");
                DrawDemoLog();
            }

            ImGui::End();
        }

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
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
