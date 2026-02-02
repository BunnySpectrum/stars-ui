#include "tactical_panel.h"
#include "field_renderer.h"
#include <cmath>

TacticalPanel::TacticalPanel() {
    // Load ship SVG and set LCARS color overrides per tagged part
    shipSvg_.LoadFromFile("assets/ship_wireframe.svg");

    const ImU32 detail = IM_COL32(0x66, 0x88, 0xAA, 0xFF);

    shipSvg_.SetShapeColor("saucer",            kBlue);
    shipSvg_.SetShapeColor("bridge",            kOrange);
    shipSvg_.SetShapeColor("saucer-grid",       detail);
    shipSvg_.SetShapeColor("neck",              kBlue);
    shipSvg_.SetShapeColor("engineering-hull",  kBlue);
    shipSvg_.SetShapeColor("deflector",         kOrange);
    shipSvg_.SetShapeColor("pylon-left",        kBlue);
    shipSvg_.SetShapeColor("pylon-right",       kBlue);
    shipSvg_.SetShapeColor("nacelle-left",      kPurple);
    shipSvg_.SetShapeColor("nacelle-right",     kPurple);
    shipSvg_.SetShapeColor("bussard-left",      kRed);
    shipSvg_.SetShapeColor("bussard-right",     kRed);
    shipSvg_.SetShapeColor("nacelle-glow-left", detail);
    shipSvg_.SetShapeColor("nacelle-glow-right",detail);

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
    ship.drawContent = [this]() {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 cursor = ImGui::GetCursorScreenPos();

        // Approach C: blink nacelle colors between purple and red
        float t = fmodf((float)ImGui::GetTime(), 1.0f);
        ImU32 nColor = (t < 0.5f) ? kPurple : kRed;
        shipSvg_.SetShapeColor("nacelle-left", nColor);
        shipSvg_.SetShapeColor("nacelle-right", nColor);

        shipSvg_.Draw(cursor, avail, kBlue, 1.5f);

        // Approach A: pulsing translucent ellipse overlay on saucer
        ImDrawList* dl = ImGui::GetWindowDrawList();
        float pulse = (sinf((float)ImGui::GetTime() * 4.0f) + 1.0f) * 0.5f;
        ImVec4 r = shipSvg_.GetShapeBounds("saucer", cursor, avail);
        if (r.z > 0) {
            ImU32 glow = IM_COL32(0x99, 0x99, 0xFF, (int)(pulse * 40));
            float cx = r.x + r.z * 0.5f;
            float cy = r.y + r.w * 0.5f;
            dl->AddEllipseFilled(ImVec2(cx, cy), ImVec2(r.z * 0.5f, r.w * 0.5f), glow);
        }

        ImGui::Dummy(avail);
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
