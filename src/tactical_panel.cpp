#include "tactical_panel.h"
#include "field_renderer.h"

#include <cmath>

// Draw a top-down wireframe starship schematic, centered and scaled to fit.
static void DrawShipWireframe() {
    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    float cx = cursor.x + avail.x * 0.5f;
    float cy = cursor.y + avail.y * 0.5f;
    // Scale to fit, ship is defined in [-1,1] range
    float scale = (avail.x < avail.y ? avail.x : avail.y) * 0.40f;

    auto pt = [&](float x, float y) -> ImVec2 {
        return ImVec2(cx + x * scale, cy + y * scale);
    };

    const ImU32 hull  = kBlue;
    const ImU32 nacelle = kPurple;
    const ImU32 detail = IM_COL32(0x66, 0x88, 0xAA, 0xFF);
    const float thick = 1.5f;

    // === Saucer section (ellipse, top-down view) ===
    int segments = 48;
    float saucerRX = 0.45f;
    float saucerRY = 0.35f;
    float saucerCY = -0.45f;
    for (int i = 0; i < segments; i++) {
        float a0 = (float)i / segments * 2.0f * (float)M_PI;
        float a1 = (float)(i + 1) / segments * 2.0f * (float)M_PI;
        dl->AddLine(
            pt(cosf(a0) * saucerRX, saucerCY + sinf(a0) * saucerRY),
            pt(cosf(a1) * saucerRX, saucerCY + sinf(a1) * saucerRY),
            hull, thick);
    }
    // Saucer bridge dot
    dl->AddCircleFilled(pt(0.0f, saucerCY), 4.0f, kOrange);

    // === Neck (connects saucer to engineering hull) ===
    float neckW = 0.06f;
    float neckTop = saucerCY + saucerRY * 0.6f;
    float neckBot = 0.10f;
    dl->AddLine(pt(-neckW, neckTop), pt(-neckW, neckBot), hull, thick);
    dl->AddLine(pt( neckW, neckTop), pt( neckW, neckBot), hull, thick);

    // === Engineering hull (elongated ellipse) ===
    float engRX = 0.12f;
    float engRY = 0.35f;
    float engCY = 0.40f;
    for (int i = 0; i < segments; i++) {
        float a0 = (float)i / segments * 2.0f * (float)M_PI;
        float a1 = (float)(i + 1) / segments * 2.0f * (float)M_PI;
        dl->AddLine(
            pt(cosf(a0) * engRX, engCY + sinf(a0) * engRY),
            pt(cosf(a1) * engRX, engCY + sinf(a1) * engRY),
            hull, thick);
    }
    // Deflector dish
    dl->AddCircle(pt(0.0f, engCY + engRY - 0.05f), 6.0f, kOrange, 16, thick);

    // === Nacelle pylons ===
    float pylonY = 0.20f;
    float pylonOuterX = 0.55f;
    dl->AddLine(pt(-engRX, pylonY), pt(-pylonOuterX, pylonY - 0.15f), hull, thick);
    dl->AddLine(pt( engRX, pylonY), pt( pylonOuterX, pylonY - 0.15f), hull, thick);

    // === Nacelles (long rectangles) ===
    float nacW  = 0.06f;
    float nacH  = 0.50f;
    float nacCY = pylonY - 0.15f - nacH * 0.35f;
    for (int side = -1; side <= 1; side += 2) {
        float nx = (float)side * pylonOuterX;
        dl->AddLine(pt(nx - nacW, nacCY - nacH * 0.5f), pt(nx + nacW, nacCY - nacH * 0.5f), nacelle, thick);
        dl->AddLine(pt(nx + nacW, nacCY - nacH * 0.5f), pt(nx + nacW, nacCY + nacH * 0.5f), nacelle, thick);
        dl->AddLine(pt(nx + nacW, nacCY + nacH * 0.5f), pt(nx - nacW, nacCY + nacH * 0.5f), nacelle, thick);
        dl->AddLine(pt(nx - nacW, nacCY + nacH * 0.5f), pt(nx - nacW, nacCY - nacH * 0.5f), nacelle, thick);
        // Bussard collector (front cap)
        dl->AddCircleFilled(pt(nx, nacCY - nacH * 0.5f), 5.0f, kRed);
        // Glow line down the nacelle
        dl->AddLine(pt(nx, nacCY - nacH * 0.35f), pt(nx, nacCY + nacH * 0.40f), detail, 1.0f);
    }

    // === Grid lines on saucer (detail) ===
    dl->AddLine(pt(-saucerRX * 0.7f, saucerCY), pt(saucerRX * 0.7f, saucerCY), detail, 1.0f);
    dl->AddLine(pt(0.0f, saucerCY - saucerRY * 0.7f), pt(0.0f, saucerCY + saucerRY * 0.7f), detail, 1.0f);

    // Reserve the space so ImGui layout advances past the drawing
    ImGui::Dummy(avail);
}

TacticalPanel::TacticalPanel() {
    // TACTICAL view
    PanelView tactical;
    tactical.name = "TACTICAL";
    tactical.buttonColor = kPurple;
    tactical.drawContent = []() {
        DrawFieldsForView("TACTICAL");
        DrawGraphsForView("TACTICAL");
    };
    views.push_back(std::move(tactical));

    // SHIP view
    PanelView ship;
    ship.name = "SHIP";
    ship.buttonColor = kTan;
    ship.drawContent = []() {
        DrawShipWireframe();
    };
    views.push_back(std::move(ship));

    // ENVIRON view
    PanelView environ;
    environ.name = "ENVIRON";
    environ.buttonColor = kBlue;
    environ.drawContent = []() {
        DrawFieldsForView("ENVIRON");
        DrawGraphsForView("ENVIRON");
    };
    environ.optionGroups = {
        { { {"ALL DECKS", kBlue, nullptr}, {"BRIDGE", kBlue, nullptr}, {"ENGINEERING", kBlue, nullptr} } },
        { { {"ALERTS", kRed, nullptr} } }
    };
    views.push_back(std::move(environ));

    // POWER view
    PanelView power;
    power.name = "POWER";
    power.buttonColor = kTan;
    power.drawContent = []() {
        DrawFieldsForView("POWER");
        DrawGraphsForView("POWER");
    };
    power.optionGroups = {
        { { {"OVERVIEW", kTan, nullptr}, {"GRID MAP", kTan, nullptr} } },
        { { {"DIVERT", kRed, nullptr}, {"BALANCE", kBlue, nullptr} } }
    };
    views.push_back(std::move(power));

    // RADIO view
    PanelView radio;
    radio.name = "RADIO";
    radio.buttonColor = kOrange;
    radio.drawContent = []() {
        DrawFieldsForView("RADIO");
        DrawGraphsForView("RADIO");
    };
    radio.optionGroups = {
        { { {"ALL FREQ", kOrange, nullptr}, {"SUBSPACE", kOrange, nullptr}, {"EM BAND", kOrange, nullptr} } },
        { { {"HAIL", kBlue, nullptr}, {"ENCRYPT", kRed, nullptr} } }
    };
    views.push_back(std::move(radio));
}

const char* TacticalPanel::GetTitle() const {
    return "LCARS TACTICAL DISPLAY 47";
}

const std::vector<PanelView>& TacticalPanel::GetViews() const {
    return views;
}
