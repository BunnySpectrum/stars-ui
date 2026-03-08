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
#include "layout_loader.h"

static void glfwErrorCallback(int error, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

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

    GLFWwindow* window = glfwCreateWindow(1280, 800, "MK1 - STARS Layout Engine", nullptr, nullptr);
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

    // Load fonts (after backend init so Build() is handled correctly)
    io.Fonts->AddFontFromFileTTF("poc/fonts/Antonio-Regular.ttf", 20.0f);
    ImFont* aurebeshFont = io.Fonts->AddFontFromFileTTF("poc/fonts/AurebeshAF-Canon.ttf", 24.0f);

    // Store Aurebesh font for use in chrome rendering
    SetAurebeshFont(aurebeshFont);

    ApplySTARSTheme();

    std::printf("MK1: STARS Layout Engine - Phase 1\n");
    std::printf("Press 'A' to toggle antialiasing\n");

    // Global antialiasing flag
    bool enableAntialiasing = true;

    // Load and parse TOML layout
    const char* layoutPath = "mk1/layouts/demo.toml";
    std::printf("Loading layout from: %s\n", layoutPath);
    LayoutLoadResult loadResult = LoadLayoutFromFile(layoutPath);

    if (!loadResult.success) {
        std::fprintf(stderr, "Failed to load layout: %s\n", loadResult.error_message.c_str());
        // Continue with hardcoded panel anyway
    } else {
        std::printf("Layout loaded successfully!\n");
        PrintScreenModel(loadResult.model);
    }

    std::printf("\nRendering hardcoded panel with chrome\n");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Toggle antialiasing with 'A' key
        if (ImGui::IsKeyPressed(ImGuiKey_A)) {
            enableAntialiasing = !enableAntialiasing;
            std::printf("Antialiasing: %s\n", enableAntialiasing ? "ON" : "OFF");
        }

        int winW, winH;
        glfwGetWindowSize(window, &winW, &winH);
        float W = (float)winW;
        float H = (float)winH;

        // Set global antialiasing flag for draw list
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        if (!enableAntialiasing) {
            drawList->Flags &= ~ImDrawListFlags_AntiAliasedFill;
            drawList->Flags &= ~ImDrawListFlags_AntiAliasedLines;
        } else {
            drawList->Flags |= ImDrawListFlags_AntiAliasedFill;
            drawList->Flags |= ImDrawListFlags_AntiAliasedLines;
        }

        // Global chrome
        DrawGlobalTopBar(W);
        DrawGlobalBottomBar(W, H);

        // Panel layout constants
        const float topBarH    = 40.0f;
        const float bottomBarH = 30.0f;
        const float panelGap   = 10.0f;

        float panelTop = topBarH;
        float panelAreaH = H - topBarH - bottomBarH;

        // Draw a single hardcoded panel
        float px = 0.0f;
        float py = panelTop;
        float pw = W;
        float ph = panelAreaH;

        ImVec4 contentRect = DrawPanelChrome(
            "DEMO PANEL",      // title
            "OVR",             // view name
            px, py, pw, ph,
            HOrientation::Left,
            VOrientation::Top
        );

        // Create ImGui window at the content rect
        float cx = contentRect.x;
        float cy = contentRect.y;
        float cw = contentRect.z;
        float ch = contentRect.w;

        ImGui::SetNextWindowPos(ImVec2(cx, cy));
        ImGui::SetNextWindowSize(ImVec2(cw, ch));
        ImGui::Begin("##DemoPanel", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoBringToFrontOnFocus
        );

        // Placeholder content
        ImGui::Text("Phase 1: STARS chrome rendering working!");
        ImGui::Separator();
        ImGui::Text("Next steps:");
        ImGui::BulletText("Load and parse TOML layout files");
        ImGui::BulletText("Implement socket connection and data store");
        ImGui::BulletText("Add basic widgets (stat, progress_bar, table)");

        ImGui::End();

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
