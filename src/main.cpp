#include <SDL.h>
#include <cstdio>
#include <memory>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include "lcars.h"
#include "tactical_panel.h"

int main(int, char**) {
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
        "LCARS Dashboard",
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

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init(glslVersion);

    ApplyLCARSTheme();

    auto panel = std::make_unique<TacticalPanel>();

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

        // Get window dimensions for frame drawing
        int winW, winH;
        SDL_GetWindowSize(window, &winW, &winH);
        float W = (float)winW;
        float H = (float)winH;

        // Content area (right of sidebar, below top bar)
        const float contentX = 170.0f;
        const float contentY = 130.0f;
        const float contentW = W - contentX - 20.0f;
        const float contentH = H - contentY - 50.0f;
        const float divBarH  = 30.0f;
        const float panelGap = 10.0f;
        const float upperH   = contentH / 3.0f - divBarH * 0.5f - panelGap;
        const float dividerY = contentY + upperH + panelGap;
        const float lowerY   = dividerY + divBarH + panelGap;
        const float lowerH   = H - lowerY - 50.0f;

        DrawLCARSFrame(W, H, dividerY);

        // Draw title text in the top bar
        {
            ImDrawList* dl = ImGui::GetBackgroundDrawList();
            const char* title = panel->GetTitle();
            const float sidebarW = 120.0f;
            const float elbowH  = 80.0f;
            const float topBarH  = 40.0f;
            ImVec2 textSize = ImGui::CalcTextSize(title);
            dl->AddText(
                ImVec2(sidebarW + elbowH + 20, (topBarH - textSize.y) * 0.5f),
                IM_COL32(0, 0, 0, 255),
                title
            );
        }

        // --- Upper-third panel ---
        ImGui::SetNextWindowPos(ImVec2(contentX, contentY));
        ImGui::SetNextWindowSize(ImVec2(contentW, upperH));
        ImGui::Begin("##UpperPanel", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoBringToFrontOnFocus
        );
        panel->DrawUpper();
        ImGui::End();

        // --- Lower two-thirds panel ---
        ImGui::SetNextWindowPos(ImVec2(contentX, lowerY));
        ImGui::SetNextWindowSize(ImVec2(contentW, lowerH));
        ImGui::Begin("##MainContent", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoBringToFrontOnFocus
        );
        panel->DrawLower();
        ImGui::End();

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
    panel.reset();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
