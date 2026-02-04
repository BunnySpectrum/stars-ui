#include <SDL.h>
#include <cstdio>
#include <memory>
#include <vector>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include "stars.h"
#include "field_store.h"
#include "db_reader.h"
#include "info_panel.h"
#include "tactical_panel.h"

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    // OpenGL attributes
#ifdef __APPLE__
    const char* glslVersion = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    const char* glslVersion = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window = SDL_CreateWindow(
        "STARS Dashboard",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 800,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1); // vsync

    // ImGui + ImPlot setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // don't save imgui.ini

    // Load Antonio font
    io.Fonts->AddFontFromFileTTF("fonts/Antonio-Regular.ttf", 20.0f);
    io.Fonts->Build();

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init(glslVersion);

    ApplySTARSTheme();

    // Database setup
    const char* dbPath = (argc > 1) ? argv[1] : nullptr;
    if (dbPath) {
        if (g_dbReader.Open(dbPath)) {
            g_useDatabase = true;
            std::printf("Database mode: %s\n", dbPath);
        } else {
            std::fprintf(stderr, "Failed to open database, using demo data\n");
        }
    } else {
        std::printf("No database specified, using demo data\n");
    }

    // Panels listed in screen order (top to bottom)
    std::vector<std::unique_ptr<STARSPanel>> panels;
    panels.push_back(std::make_unique<InfoPanel>());
    panels.push_back(std::make_unique<TacticalPanel>());

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                running = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        UpdateFieldStore();
        if (g_useDatabase) g_dbReader.PollIfNeeded();

        int winW, winH;
        SDL_GetWindowSize(window, &winW, &winH);
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

            const auto& views = panels[i]->GetViews();
            int av = panels[i]->activeView;
            if (av >= 0 && av < (int)views.size() && views[av].drawContent) {
                views[av].drawContent();
            }

            ImGui::End();
            py += ph + panelGap;
        }

        // Render
        ImGui::Render();
        int fbW, fbH;
        SDL_GL_GetDrawableSize(window, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    g_dbReader.Close();
    panels.clear();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
