#pragma once

#include "../stars.h"
#include "../panel_common.h"
#include "../views/view_info.h"

class InfoPanel : public STARSPanel {
public:
    InfoPanel() : STARSPanel("STARS INFORMATION DISPLAY 01", 1.0f) {
        views_.push_back(CreatePanelViewFromViewInfo(kInfoViewInfo, svgRenderers_));
    }
};
