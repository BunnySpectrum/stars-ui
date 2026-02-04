#pragma once

#include "stars.h"

class InfoPanel : public STARSPanel {
    std::vector<PanelView> views;
public:
    InfoPanel();
    const char* GetTitle() const override;
    const std::vector<PanelView>& GetViews() const override;
};
