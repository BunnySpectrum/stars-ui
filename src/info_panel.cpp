#include "info_panel.h"
#include "field_renderer.h"

InfoPanel::InfoPanel() {
    PanelView info;
    info.name = "INFO";
    info.buttonColor = kBlue;
    info.drawContent = []() {
        DrawFieldsForView("INFO");
    };
    views.push_back(std::move(info));
}

const char* InfoPanel::GetTitle() const {
    return "STARS INFORMATION DISPLAY 01";
}

const std::vector<PanelView>& InfoPanel::GetViews() const {
    return views;
}
