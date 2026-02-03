#include "svg_renderer.h"

#include "nanosvg.h"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <sstream>
#include <regex>

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
    textElements_.clear();

    image_ = nsvgParseFromFile(filename, "px", 96.0f);
    if (image_) {
        ParseTextElements(filename);
    }
    return image_ != nullptr;
}

// Simple XML attribute parser
static std::string GetAttr(const std::string& tag, const char* attr) {
    std::string pattern = std::string(attr) + "=\"";
    size_t pos = tag.find(pattern);
    if (pos == std::string::npos) return "";
    pos += pattern.length();
    size_t end = tag.find('"', pos);
    if (end == std::string::npos) return "";
    return tag.substr(pos, end - pos);
}

// Parse hex color string to ImU32
static ImU32 ParseColor(const std::string& colorStr) {
    if (colorStr.empty() || colorStr[0] != '#') return IM_COL32(255, 255, 255, 255);
    unsigned int hex = 0;
    std::stringstream ss;
    ss << std::hex << colorStr.substr(1);
    ss >> hex;
    if (colorStr.length() == 7) { // #RRGGBB
        return IM_COL32((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF, 255);
    }
    return IM_COL32(255, 255, 255, 255);
}

void SvgRenderer::ParseTextElements(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Parse viewBox to get coordinate offset
    // Format: viewBox="minX minY width height"
    viewBoxX_ = 0.0f;
    viewBoxY_ = 0.0f;
    std::regex viewBoxRegex("viewBox=\"([^\"]*)\"");
    std::smatch vbMatch;
    if (std::regex_search(content, vbMatch, viewBoxRegex)) {
        std::string vb = vbMatch[1].str();
        float minX = 0, minY = 0, w = 0, h = 0;
        if (std::sscanf(vb.c_str(), "%f %f %f %f", &minX, &minY, &w, &h) >= 2) {
            viewBoxX_ = minX;
            viewBoxY_ = minY;
        }
    }

    // Find all <text ...>content</text> elements
    std::regex textRegex(R"(<text\s+([^>]*)>([^<]*)</text>)");
    std::smatch match;
    std::string::const_iterator searchStart(content.cbegin());

    while (std::regex_search(searchStart, content.cend(), match, textRegex)) {
        std::string attrs = match[1].str();
        std::string textContent = match[2].str();

        SvgText text;
        text.content = textContent;
        text.id = GetAttr(attrs, "id");

        // Parse position and apply viewBox offset
        std::string xStr = GetAttr(attrs, "x");
        std::string yStr = GetAttr(attrs, "y");
        text.x = (xStr.empty() ? 0.0f : std::stof(xStr)) - viewBoxX_;
        text.y = (yStr.empty() ? 0.0f : std::stof(yStr)) - viewBoxY_;

        // Parse font size
        std::string fontSize = GetAttr(attrs, "font-size");
        text.fontSize = fontSize.empty() ? 12.0f : std::stof(fontSize);

        // Parse text-anchor
        std::string anchor = GetAttr(attrs, "text-anchor");
        if (anchor == "middle") text.anchor = 1;
        else if (anchor == "end") text.anchor = 2;
        else text.anchor = 0; // start

        // Parse color from stroke attribute
        std::string stroke = GetAttr(attrs, "stroke");
        text.color = stroke.empty() ? IM_COL32(255, 255, 255, 255) : ParseColor(stroke);

        textElements_.push_back(text);
        searchStart = match.suffix().first;
    }
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

    // NOTE: Text rendering disabled due to coordinate transform issues.
    // See bug-svg-text-location.md for details.
    // Use SvgBindingDef system to overlay data labels on shapes instead.
#if 0
    // Render text elements (nanosvg doesn't support text, so we parse and render separately)
    ImFont* font = ImGui::GetFont();
    for (const SvgText& text : textElements_) {
        if (text.content.empty()) continue;

        // Check for color override by id
        ImU32 textColor = text.color;
        if (!text.id.empty()) {
            auto it = colorOverrides_.find(text.id);
            if (it != colorOverrides_.end()) {
                textColor = it->second;
            }
        }

        // Scale font size
        float scaledFontSize = text.fontSize * scale;
        if (scaledFontSize < 8.0f) scaledFontSize = 8.0f;  // minimum readable size

        // Calculate text position
        ImVec2 pos = transform(text.x, text.y);

        // Adjust for text-anchor
        ImVec2 textSize = font->CalcTextSizeA(scaledFontSize, FLT_MAX, 0.0f, text.content.c_str());
        if (text.anchor == 1) { // middle
            pos.x -= textSize.x * 0.5f;
        } else if (text.anchor == 2) { // end
            pos.x -= textSize.x;
        }

        // Adjust Y to account for baseline (SVG y is baseline, ImGui y is top)
        pos.y -= scaledFontSize * 0.8f;

        dl->AddText(font, scaledFontSize, pos, textColor, text.content.c_str());
    }
#endif
}

ImVec4 SvgRenderer::GetShapeBounds(const char* id, ImVec2 origin, ImVec2 size) const {
    if (!image_ || !id || !id[0]) return ImVec4(0, 0, 0, 0);

    float svgW = image_->width;
    float svgH = image_->height;
    if (svgW <= 0 || svgH <= 0) return ImVec4(0, 0, 0, 0);

    // Same uniform-scale transform as Draw()
    float scaleX = size.x / svgW;
    float scaleY = size.y / svgH;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    float offsetX = origin.x + (size.x - svgW * scale) * 0.5f;
    float offsetY = origin.y + (size.y - svgH * scale) * 0.5f;

    for (NSVGshape* shape = image_->shapes; shape != nullptr; shape = shape->next) {
        if (strcmp(shape->id, id) == 0) {
            float x = offsetX + shape->bounds[0] * scale;
            float y = offsetY + shape->bounds[1] * scale;
            float w = (shape->bounds[2] - shape->bounds[0]) * scale;
            float h = (shape->bounds[3] - shape->bounds[1]) * scale;
            return ImVec4(x, y, w, h);
        }
    }

    return ImVec4(0, 0, 0, 0);
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
