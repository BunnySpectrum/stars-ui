#include <GLFW/glfw3.h>
#include <cstdio>
#include <memory>
#include <vector>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "implot/implot.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include "stars.h"
#include "field_store.h"
#include "db_reader.h"
#include "panels/info_panel.h"
#include "panels/tactical_panel.h"

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

    GLFWwindow* window = glfwCreateWindow(1280, 800, "STARS Dashboard", nullptr, nullptr);
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

    // Database setup (required)
    if (argc < 2) {
        std::fprintf(stderr, "Usage: %s <database.db>\n", argv[0]);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    const char* dbPath = argv[1];
    if (!g_dbReader.Open(dbPath)) {
        std::fprintf(stderr, "Failed to open database: %s\n", dbPath);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    std::printf("Database: %s\n", dbPath);

    // Panels listed in screen order (top to bottom)
    std::vector<std::unique_ptr<STARSPanel>> panels;
    panels.push_back(std::make_unique<InfoPanel>());
    panels.push_back(std::make_unique<TacticalPanel>());

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        g_dbReader.PollIfNeeded();

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
        int nPanels = (int)panels.size();

        // Distribute vertical space by each panel's height weight
        float totalWeight = 0.0f;
        for (int i = 0; i < nPanels; i++)
            totalWeight += panels[i]->GetHeightWeight();
        float usableH = panelAreaH - panelGap * (nPanels - 1);

        float py = panelTop;
        for (int i = 0; i < nPanels; i++) {
            float ph = usableH * (panels[i]->GetHeightWeight() / totalWeight);
            VOrientation vOrient = (i == 0) ? VOrientation::Bottom : VOrientation::Top;

            ImVec4 contentRect = DrawPanelChrome(*panels[i], 0, py, W, ph, vOrient);

            // Create ImGui window at the content rect
            float cx = contentRect.x;
            float cy = contentRect.y;
            float cw = contentRect.z;
            float ch = contentRect.w;

            char winName[32];
            snprintf(winName, sizeof(winName), "##Panel%d", i);

            ImGui::SetNextWindowPos(ImVec2(cx, cy));
            ImGui::SetNextWindowSize(ImVec2(cw, ch));
            ImGui::Begin(winName, nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoBringToFrontOnFocus
            );

            auto& views = panels[i]->GetViews();
            int av = panels[i]->activeView;
            if (av >= 0 && av < (int)views.size()) {
                auto& view = views[av];
                // Update the active view's data (handles both demo and database modes)
                if (view.updateFields && view.fields) {
                    view.updateFields(*view.fields, view.graphBufs);
                }
                if (view.drawContent && view.fields) {
                    view.drawContent(*view.fields, view.graphBufs);
                }
            }

            ImGui::End();
            py += ph + panelGap;
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
    g_dbReader.Close();
    panels.clear();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
