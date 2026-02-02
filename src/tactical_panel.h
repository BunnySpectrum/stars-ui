#pragma once

#include "lcars.h"

class TacticalPanel : public LCARSPanel {
    std::vector<PanelView> views;
public:
    TacticalPanel();
    const char* GetTitle() const override;
    float GetHeightWeight() const override { return 2.0f; }
    const std::vector<PanelView>& GetViews() const override;
};
