#include <SDL.h>
#include <cmath>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

// LCARS color palette
static const ImU32 kOrange  = IM_COL32(0xFF, 0x99, 0x33, 0xFF);
static const ImU32 kPurple  = IM_COL32(0xCC, 0x99, 0xCC, 0xFF);
static const ImU32 kBlue    = IM_COL32(0x99, 0x99, 0xFF, 0xFF);
static const ImU32 kTan     = IM_COL32(0xFF, 0xCC, 0x99, 0xFF);
static const ImU32 kRed     = IM_COL32(0xCC, 0x66, 0x66, 0xFF);
static const ImU32 kBeige   = IM_COL32(0xFF, 0xDD, 0xBB, 0xFF);

static ImVec4 U32ToVec4(ImU32 c) {
    return ImVec4(
        ((c >>  0) & 0xFF) / 255.0f,
        ((c >>  8) & 0xFF) / 255.0f,
        ((c >> 16) & 0xFF) / 255.0f,
        ((c >> 24) & 0xFF) / 255.0f
    );
}

static void ApplyLCARSTheme() {
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

static void DrawLCARSFrame(float W, float H) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    // Dimensions
    const float topBarH    = 40.0f;
    const float sidebarW   = 120.0f;
    const float elbowH     = 80.0f;
    const float bottomBarH = 30.0f;
    const float gap        = 6.0f;

    // Top bar (right of elbow)
    dl->AddRectFilled(
        ImVec2(sidebarW + elbowH, 0),
        ImVec2(W, topBarH),
        kOrange, 0.0f, 0
    );
    // Rounded right end of top bar
    dl->AddRectFilled(
        ImVec2(W - 20, 0),
        ImVec2(W, topBarH),
        kOrange, 20.0f, ImDrawFlags_RoundCornersRight
    );

    // Elbow block (the filled rectangle part)
    dl->AddRectFilled(
        ImVec2(0, 0),
        ImVec2(sidebarW + elbowH, topBarH + elbowH),
        kOrange, 0.0f, 0
    );

    // Cut out the inner arc of the elbow with a black quarter-circle
    const ImVec2 arcCenter(sidebarW + elbowH, topBarH + elbowH);
    const int arcSegments = 32;
    // Build a filled polygon: arc + corner to carve out the curve
    ImVector<ImVec2> arcPoints;
    for (int i = 0; i <= arcSegments; i++) {
        float angle = (float)M_PI * 1.0f + ((float)M_PI * 0.5f) * ((float)i / (float)arcSegments);
        arcPoints.push_back(ImVec2(
            arcCenter.x + cosf(angle) * elbowH,
            arcCenter.y + sinf(angle) * elbowH
        ));
    }
    arcPoints.push_back(arcCenter);
    dl->AddConvexPolyFilled(arcPoints.Data, arcPoints.Size, IM_COL32(0, 0, 0, 255));

    // Sidebar blocks below elbow
    const float sideTop = topBarH + elbowH + gap;
    const float blockH  = (H - sideTop - bottomBarH - gap * 4) / 4.0f;
    const ImU32 sideColors[] = { kPurple, kBlue, kTan, kOrange };
    for (int i = 0; i < 4; i++) {
        float y0 = sideTop + i * (blockH + gap);
        float y1 = y0 + blockH;
        dl->AddRectFilled(
            ImVec2(0, y0), ImVec2(sidebarW, y1),
            sideColors[i], 20.0f, ImDrawFlags_RoundCornersLeft
        );
    }

    // Bottom bar
    dl->AddRectFilled(
        ImVec2(sidebarW + gap, H - bottomBarH),
        ImVec2(W, H),
        kTan, 0.0f, 0
    );
    // Rounded left end of bottom bar
    dl->AddRectFilled(
        ImVec2(sidebarW + gap, H - bottomBarH),
        ImVec2(sidebarW + gap + 20, H),
        kTan, 20.0f, ImDrawFlags_RoundCornersLeft
    );

    // Title text in the top bar
    const char* title = "LCARS TACTICAL DISPLAY 47";
    ImVec2 textSize = ImGui::CalcTextSize(title);
    dl->AddText(
        ImVec2(sidebarW + elbowH + 20, (topBarH - textSize.y) * 0.5f),
        IM_COL32(0, 0, 0, 255),
        title
    );
}

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

        DrawLCARSFrame(W, H);

        // Content area (right of sidebar, below top bar)
        const float contentX = 140.0f;
        const float contentY = 130.0f;
        const float contentW = W - contentX - 20.0f;
        const float contentH = H - contentY - 50.0f;

        ImGui::SetNextWindowPos(ImVec2(contentX, contentY));
        ImGui::SetNextWindowSize(ImVec2(contentW, contentH));
        ImGui::Begin("##MainContent", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoBringToFrontOnFocus
        );

        // --- System Status Panel ---
        ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kTan));
        ImGui::SeparatorText("SYSTEM STATUS");
        ImGui::PopStyleColor();

        ImGui::Columns(2, nullptr, false);
        ImGui::SetColumnWidth(0, 200.0f);

        ImGui::TextColored(U32ToVec4(kPurple), "WARP CORE");
        ImGui::TextColored(U32ToVec4(kPurple), "SHIELDS");
        ImGui::TextColored(U32ToVec4(kPurple), "HULL INTEGRITY");
        ImGui::TextColored(U32ToVec4(kPurple), "LIFE SUPPORT");

        ImGui::NextColumn();

        ImGui::TextColored(U32ToVec4(kBeige), "ONLINE  -  OUTPUT 98.7%%");
        ImGui::TextColored(U32ToVec4(kBeige), "ACTIVE  -  STRENGTH 100%%");
        ImGui::TextColored(U32ToVec4(kBeige), "NOMINAL -  97.3%%");
        ImGui::TextColored(U32ToVec4(kBeige), "NOMINAL -  ALL DECKS");

        ImGui::Columns(1);
        ImGui::Spacing();
        ImGui::Spacing();

        // --- Sensor Telemetry Panel ---
        ImGui::PushStyleColor(ImGuiCol_Text, U32ToVec4(kTan));
        ImGui::SeparatorText("SENSOR TELEMETRY");
        ImGui::PopStyleColor();

        // Animated sine/cosine data
        float t = (float)ImGui::GetTime();
        static const int N = 256;
        static float xs[N], ys1[N], ys2[N];
        for (int i = 0; i < N; i++) {
            xs[i]  = (float)i / (float)(N - 1) * 10.0f;
            ys1[i] = sinf(xs[i] + t) * 0.5f + 0.5f * sinf(xs[i] * 2.3f + t * 1.7f);
            ys2[i] = cosf(xs[i] * 0.7f + t * 0.8f) * 0.8f;
        }

        float plotW = contentW - 20.0f;
        float plotH = contentH - ImGui::GetCursorPosY() + contentY - 20.0f;
        if (plotH < 100.0f) plotH = 100.0f;

        ImPlot::PushStyleColor(ImPlotCol_Line, U32ToVec4(kOrange));
        if (ImPlot::BeginPlot("##Telemetry", ImVec2(plotW, plotH))) {
            ImPlot::SetupAxes("FREQUENCY", "AMPLITUDE");
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, 10, ImPlotCond_Once);
            ImPlot::SetupAxisLimits(ImAxis_Y1, -1.5, 1.5, ImPlotCond_Once);

            ImPlot::PushStyleColor(ImPlotCol_Line, U32ToVec4(kOrange));
            ImPlot::PlotLine("Subspace Band", xs, ys1, N);
            ImPlot::PopStyleColor();

            ImPlot::PushStyleColor(ImPlotCol_Line, U32ToVec4(kBlue));
            ImPlot::PlotLine("EM Spectrum", xs, ys2, N);
            ImPlot::PopStyleColor();

            ImPlot::EndPlot();
        }
        ImPlot::PopStyleColor();

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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
