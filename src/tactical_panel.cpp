#include "tactical_panel.h"
#include "field_defs.h"
#include "field_renderer.h"
#include <cmath>
#include <cstring>

TacticalPanel::TacticalPanel() {
    // Load SVGs from the view table
    for (int i = 0; i < kNumSvgViews; i++) {
        if (std::strcmp(kSvgViews[i].view, "SHIP") == 0) {
            shipSvg_.LoadFromFile(kSvgViews[i].svgPath);
        } else if (std::strcmp(kSvgViews[i].view, "MAP") == 0) {
            mapSvg_.LoadFromFile(kSvgViews[i].svgPath);
        } else if (std::strcmp(kSvgViews[i].view, "RF") == 0) {
            rfSvg_.LoadFromFile(kSvgViews[i].svgPath);
        }
    }

    const ImU32 detail = IM_COL32(0x66, 0x88, 0xAA, 0xFF);

    shipSvg_.SetShapeColor("shield",            kBlue);
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

    // Set default colors for map SVG
    mapSvg_.SetShapeColor("geo-orbit",          detail);
    mapSvg_.SetShapeColor("earth",              kBlue);
    mapSvg_.SetShapeColor("earth-grid",         detail);
    mapSvg_.SetShapeColor("equator",            kBlue);
    mapSvg_.SetShapeColor("ground-station",     kOrange);
    mapSvg_.SetShapeColor("goes16",             kBlue);
    mapSvg_.SetShapeColor("link-line",          kOrange);
    mapSvg_.SetShapeColor("signal-cone",        detail);
    mapSvg_.SetShapeColor("subsatellite-point", kPurple);
    mapSvg_.SetShapeColor("coverage-arc",       kPurple);
    mapSvg_.SetShapeColor("lon-grid",           detail);

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

        // 1. Data-driven color overrides from bindings
        ApplySvgBindingColors(shipSvg_, "SHIP");

        // 2. Existing nacelle blink (runs after bindings, so it wins for nacelles)
        float t = fmodf((float)ImGui::GetTime(), 1.0f);
        ImU32 nColor = (t < 0.5f) ? kPurple : kRed;
        shipSvg_.SetShapeColor("nacelle-left", nColor);
        shipSvg_.SetShapeColor("nacelle-right", nColor);

        // 3. Draw SVG
        shipSvg_.Draw(cursor, avail, kBlue, 1.5f);

        // 4. Data-bound text labels
        DrawSvgBindingLabels(shipSvg_, "SHIP", cursor, avail);

        // 5. Existing saucer glow overlay
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

    // MAP view (GOES-16 orbital visualization)
    PanelView map;
    map.name = "MAP";
    map.buttonColor = kBlue;
    map.drawContent = [this]() {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 cursor = ImGui::GetCursorScreenPos();

        // Apply data-driven color overrides from bindings
        ApplySvgBindingColors(mapSvg_, "MAP");

        // Animate the satellite pulse
        float pulse = (sinf((float)ImGui::GetTime() * 3.0f) + 1.0f) * 0.5f;
        ImU32 satColor = IM_COL32(0x99, 0x99, 0xFF, 128 + (int)(pulse * 127));
        mapSvg_.SetShapeColor("goes16", satColor);

        // Draw SVG
        mapSvg_.Draw(cursor, avail, kBlue, 1.5f);

        // Draw data-bound text labels
        DrawSvgBindingLabels(mapSvg_, "MAP", cursor, avail);

        ImGui::Dummy(avail);
    };
    views.push_back(std::move(map));

    // RF view (RF circuit schematic)
    // Set default colors for RF circuit components
    rfSvg_.SetShapeColor("oscillator-y1",  kOrange);
    rfSvg_.SetShapeColor("oscillator-y2",  kOrange);
    rfSvg_.SetShapeColor("mixer-u2",       kBlue);
    rfSvg_.SetShapeColor("mixer-u3",       kBlue);
    rfSvg_.SetShapeColor("opamp-u4",       kPurple);
    rfSvg_.SetShapeColor("dac-u1",         kTan);
    rfSvg_.SetShapeColor("antenna-ae1",    kOrange);

    PanelView rf;
    rf.name = "RF";
    rf.buttonColor = kTan;
    rf.drawContent = [this]() {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 cursor = ImGui::GetCursorScreenPos();

        // Apply data-driven color overrides from bindings
        ApplySvgBindingColors(rfSvg_, "RF");

        // Animate mixer components to show signal flow
        float t = (float)ImGui::GetTime();
        float mixerPulse = (sinf(t * 4.0f) + 1.0f) * 0.5f;
        ImU32 mixerColor = IM_COL32(0x99, 0x99, 0xFF, 128 + (int)(mixerPulse * 127));
        rfSvg_.SetShapeColor("mixer-u2", mixerColor);
        rfSvg_.SetShapeColor("mixer-u3", mixerColor);

        // Animate oscillators
        float oscPulse = (sinf(t * 6.0f) + 1.0f) * 0.5f;
        ImU32 oscColor = IM_COL32(0xFF, 0x99, 0x33, 128 + (int)(oscPulse * 127));
        rfSvg_.SetShapeColor("oscillator-y1", oscColor);
        rfSvg_.SetShapeColor("oscillator-y2", oscColor);

        // Draw SVG
        rfSvg_.Draw(cursor, avail, kBlue, 1.5f);

        // Draw data-bound text labels
        DrawSvgBindingLabels(rfSvg_, "RF", cursor, avail);

        ImGui::Dummy(avail);
    };
    views.push_back(std::move(rf));
}

const char* TacticalPanel::GetTitle() const {
    return "STARS TACTICAL DISPLAY 47";
}

const std::vector<PanelView>& TacticalPanel::GetViews() const {
    return views;
}
