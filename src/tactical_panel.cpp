#include "tactical_panel.h"
#include "field_renderer.h"

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
