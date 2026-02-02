#pragma once

#include "imgui.h"
#include <string>
#include <unordered_map>
#include <vector>

struct NSVGimage;

class SvgRenderer {
public:
    SvgRenderer() = default;
    ~SvgRenderer();

    SvgRenderer(const SvgRenderer&) = delete;
    SvgRenderer& operator=(const SvgRenderer&) = delete;

    bool LoadFromFile(const char* filename);

    void SetShapeColor(const std::string& id, ImU32 color);
    void ClearColorOverrides();

    void Draw(ImVec2 origin, ImVec2 size, ImU32 defaultColor, float thickness = 1.5f) const;

    // Returns screen-space bounding rect {x, y, w, h} for the shape with the given id,
    // using the same uniform-scale transform as Draw(). Returns {0,0,0,0} if not found.
    ImVec4 GetShapeBounds(const char* id, ImVec2 origin, ImVec2 size) const;

    std::vector<std::string> GetShapeIds() const;

private:
    NSVGimage* image_ = nullptr;
    std::unordered_map<std::string, ImU32> colorOverrides_;
};
