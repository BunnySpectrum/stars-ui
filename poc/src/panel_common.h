#pragma once

#include "stars.h"
#include "svg_renderer.h"
#include "field_store.h"
#include "views/view_common.h"

#include <map>
#include <string>

// Helper to create a PanelView from a ViewInfo
inline PanelView CreatePanelViewFromViewInfo(const ViewInfo& vi,
                                              std::map<std::string, SvgRenderer>& svgRenderers) {
    PanelView pv;
    pv.name = vi.name;
    pv.buttonColor = vi.buttonColor;

    // Convert option groups
    for (int g = 0; g < vi.optionGroupCount; g++) {
        OptionGroup og;
        for (int o = 0; o < vi.optionGroups[g].count; o++) {
            og.buttons.push_back({
                vi.optionGroups[g].options[o].label,
                vi.optionGroups[g].options[o].color,
                nullptr
            });
        }
        pv.optionGroups.push_back(std::move(og));
    }

    // Initialize SVG if this view has one
    if (vi.initSvg) {
        vi.initSvg(svgRenderers[vi.name]);
    }

    // Allocate per-view field store and graph buffers
    pv.fields = new FieldStore();
    pv.graphBufs.resize(vi.graphCount);
    pv.updateFields = vi.updateFields;

    pv.drawContent = vi.drawContent;
    return pv;
}
