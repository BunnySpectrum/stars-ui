#pragma once

#include "lcars.h"

class InfoPanel : public LCARSPanel {
    std::vector<PanelView> views;
public:
    InfoPanel();
    const char* GetTitle() const override;
    const std::vector<PanelView>& GetViews() const override;
};
