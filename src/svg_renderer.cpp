#include "svg_renderer.h"

#include "nanosvg.h"

SvgRenderer::~SvgRenderer() {
    if (image_) {
        nsvgDelete(image_);
        image_ = nullptr;
    }
}

bool SvgRenderer::LoadFromFile(const char* filename) {
    if (image_) {
        nsvgDelete(image_);
        image_ = nullptr;
    }
    image_ = nsvgParseFromFile(filename, "px", 96.0f);
    return image_ != nullptr;
}

void SvgRenderer::SetShapeColor(const std::string& id, ImU32 color) {
    colorOverrides_[id] = color;
}

void SvgRenderer::ClearColorOverrides() {
    colorOverrides_.clear();
}

void SvgRenderer::Draw(ImVec2 origin, ImVec2 size, ImU32 defaultColor, float thickness) const {
    if (!image_) return;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    float svgW = image_->width;
    float svgH = image_->height;
    if (svgW <= 0 || svgH <= 0) return;

    // Uniform scale to fit target rect, centered
    float scaleX = size.x / svgW;
    float scaleY = size.y / svgH;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    float offsetX = origin.x + (size.x - svgW * scale) * 0.5f;
    float offsetY = origin.y + (size.y - svgH * scale) * 0.5f;

    auto transform = [&](float x, float y) -> ImVec2 {
        return ImVec2(offsetX + x * scale, offsetY + y * scale);
    };

    for (NSVGshape* shape = image_->shapes; shape != nullptr; shape = shape->next) {
        if (!(shape->flags & NSVG_FLAGS_VISIBLE)) continue;

        // Determine color: override by id, else use shape's own color, else default
        ImU32 strokeColor = defaultColor;
        ImU32 fillColor = defaultColor;
        bool hasColorOverride = false;

        if (shape->id[0] != '\0') {
            auto it = colorOverrides_.find(shape->id);
            if (it != colorOverrides_.end()) {
                strokeColor = it->second;
                fillColor = it->second;
                hasColorOverride = true;
            }
        }

        if (!hasColorOverride) {
            // Use the SVG's own colors
            if (shape->stroke.type == NSVG_PAINT_COLOR) {
                unsigned int c = shape->stroke.color;
                strokeColor = IM_COL32((c & 0xFF), ((c >> 8) & 0xFF), ((c >> 16) & 0xFF), 0xFF);
            }
            if (shape->fill.type == NSVG_PAINT_COLOR) {
                unsigned int c = shape->fill.color;
                fillColor = IM_COL32((c & 0xFF), ((c >> 8) & 0xFF), ((c >> 16) & 0xFF), 0xFF);
            }
        }

        bool hasFill = (shape->fill.type != NSVG_PAINT_NONE);
        bool hasStroke = (shape->stroke.type != NSVG_PAINT_NONE);
        float strokeWidth = shape->strokeWidth * scale;
        if (strokeWidth < 1.0f) strokeWidth = thickness;

        for (NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
            int npts = path->npts;
            if (npts < 4) continue;

            // Helper: build the ImGui path from nanosvg cubic bezier points.
            // nanosvg stores points as flat floats [x0,y0, x1,y1, ...].
            // Each cubic segment uses 4 points (start shared with prev end),
            // so after the first point, every 3 points form one segment.
            auto buildPath = [&]() {
                dl->PathClear();
                dl->PathLineTo(transform(path->pts[0], path->pts[1]));
                int numSegs = (npts - 1) / 3;
                for (int seg = 0; seg < numSegs; seg++) {
                    int base = (seg * 3 + 1) * 2;
                    ImVec2 cp1 = transform(path->pts[base],     path->pts[base + 1]);
                    ImVec2 cp2 = transform(path->pts[base + 2], path->pts[base + 3]);
                    ImVec2 end = transform(path->pts[base + 4], path->pts[base + 5]);
                    dl->PathBezierCubicCurveTo(cp1, cp2, end);
                }
            };

            buildPath();

            if (hasFill) {
                dl->PathFillConvex(fillColor);
                if (hasStroke) {
                    buildPath();
                    dl->PathStroke(strokeColor, path->closed ? ImDrawFlags_Closed : ImDrawFlags_None, strokeWidth);
                }
            } else if (hasStroke) {
                dl->PathStroke(strokeColor, path->closed ? ImDrawFlags_Closed : ImDrawFlags_None, strokeWidth);
            }
        }
    }
}

std::vector<std::string> SvgRenderer::GetShapeIds() const {
    std::vector<std::string> ids;
    if (!image_) return ids;
    for (NSVGshape* shape = image_->shapes; shape != nullptr; shape = shape->next) {
        if (shape->id[0] != '\0') {
            ids.emplace_back(shape->id);
        }
    }
    return ids;
}
