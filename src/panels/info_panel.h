#pragma once

#include "../stars.h"
#include "../field_defs.h"
#include "../field_renderer.h"

class InfoPanel : public STARSPanel {
public:
    InfoPanel() : STARSPanel("STARS INFORMATION DISPLAY 01", 1.0f) {
        PanelView info;
        info.name = "INFO";
        info.buttonColor = kBlue;
        info.drawContent = []() {
            DrawFieldsForView(ViewId::Info);
        };
        views_.push_back(std::move(info));
    }
};
