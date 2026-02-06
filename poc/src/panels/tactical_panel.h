#pragma once

#include "../stars.h"
#include "../panel_common.h"
#include "../views/view_environ.h"
#include "../views/view_map.h"
#include "../views/view_power.h"
#include "../views/view_radio.h"
#include "../views/view_rf.h"
#include "../views/view_ship.h"
#include "../views/view_tactical.h"

class TacticalPanel : public STARSPanel {
public:
    TacticalPanel() : STARSPanel("STARS TACTICAL DISPLAY 47", 2.0f) {
        views_.push_back(CreatePanelViewFromViewInfo(kTacticalViewInfo, svgRenderers_));
        views_.push_back(CreatePanelViewFromViewInfo(kEnvironViewInfo, svgRenderers_));
        views_.push_back(CreatePanelViewFromViewInfo(kPowerViewInfo, svgRenderers_));
        views_.push_back(CreatePanelViewFromViewInfo(kRadioViewInfo, svgRenderers_));
        views_.push_back(CreatePanelViewFromViewInfo(kShipViewInfo, svgRenderers_));
        views_.push_back(CreatePanelViewFromViewInfo(kMapViewInfo, svgRenderers_));
        views_.push_back(CreatePanelViewFromViewInfo(kRfViewInfo, svgRenderers_));
    }
};
