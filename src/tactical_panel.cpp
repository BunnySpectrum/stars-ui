#include "tactical_panel.h"
#include "field_renderer.h"

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
        shipSvg_.Draw(cursor, avail, kBlue, 1.5f);
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
