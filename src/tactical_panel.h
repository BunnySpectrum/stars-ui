#pragma once

#include "stars.h"
#include "svg_renderer.h"

class TacticalPanel : public STARSPanel {
    std::vector<PanelView> views;
    SvgRenderer shipSvg_;
    SvgRenderer mapSvg_;
    SvgRenderer rfSvg_;
public:
    TacticalPanel();
    const char* GetTitle() const override;
    float GetHeightWeight() const override { return 2.0f; }
    const std::vector<PanelView>& GetViews() const override;
};
