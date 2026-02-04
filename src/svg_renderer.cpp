#include "svg_renderer.h"
#include "lcars.h"

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
    groupElements_.clear();

    image_ = nsvgParseFromFile(filename, "px", 96.0f);
    if (image_) {
        ParseTextElements(filename);
        ParseGroupElements(filename);
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

// Helper to safely parse a float, returns 0 on failure
static float SafeStof(const std::string& s) {
    if (s.empty()) return 0.0f;
    try {
        return std::stof(s);
    } catch (...) {
        return 0.0f;
    }
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

    // First, find all <g> elements with transforms to build a transform map
    // Format: <g id="..." transform="translate(x,y)">
    std::regex groupRegex(R"(<g\s+id=\"([^\"]+)\"[^>]*transform=\"([^\"]+)\"[^>]*>)");
    std::unordered_map<size_t, std::pair<float, float>> posToTransform;

    std::sregex_iterator groupIt(content.begin(), content.end(), groupRegex);
    std::sregex_iterator groupEnd;
    std::vector<std::tuple<size_t, size_t, float, float>> groupRanges; // start, end, tx, ty

    while (groupIt != groupEnd) {
        size_t groupStart = groupIt->position();
        std::string transform = (*groupIt)[2].str();
        float tx = 0, ty = 0;

        // Parse translate
        std::regex translateRegex(R"(translate\(\s*([+-]?\d*\.?\d+)\s*,\s*([+-]?\d*\.?\d+)\s*\))");
        std::smatch tm;
        if (std::regex_search(transform, tm, translateRegex)) {
            tx = SafeStof(tm[1].str());
            ty = SafeStof(tm[2].str());
        }

        // Find the closing </g> - simple approach: find next </g> after this position
        size_t groupEnd = content.find("</g>", groupStart);
        if (groupEnd != std::string::npos) {
            groupRanges.emplace_back(groupStart, groupEnd, tx, ty);
        }
        ++groupIt;
    }

    // Find all <text ...>content</text> elements
    std::regex textRegex(R"(<text\s+([^>]*)>([^<]*)</text>)");
    std::sregex_iterator textIt(content.begin(), content.end(), textRegex);
    std::sregex_iterator textEnd;

    while (textIt != textEnd) {
        size_t textPos = textIt->position();
        std::string attrs = (*textIt)[1].str();
        std::string textContent = (*textIt)[2].str();

        // Find which group this text belongs to (if any)
        float groupTx = 0, groupTy = 0;
        for (const auto& [start, end, tx, ty] : groupRanges) {
            if (textPos > start && textPos < end) {
                groupTx = tx;
                groupTy = ty;
                break;
            }
        }

        SvgText text;
        text.content = textContent;
        text.id = GetAttr(attrs, "id");

        // Parse position, apply group transform, then viewBox offset
        // Convert from mm to pixels (96 DPI) to match nanosvg coordinate space
        constexpr float mmToPx = 96.0f / 25.4f;  // ~3.78 px/mm
        std::string xStr = GetAttr(attrs, "x");
        std::string yStr = GetAttr(attrs, "y");
        text.x = (SafeStof(xStr) + groupTx - viewBoxX_) * mmToPx;
        text.y = (SafeStof(yStr) + groupTy - viewBoxY_) * mmToPx;

        // Parse font size (may have units like "12px" or "0.27mm")
        std::string fontSize = GetAttr(attrs, "font-size");
        float parsedSize = fontSize.empty() ? 12.0f : SafeStof(fontSize);
        // If font size is in mm (very small value < 1), scale it up significantly
        // 0.27mm in a 152mm viewBox displayed at 800px would be ~1.4px - too small
        // We want readable text, so multiply by ~25 to get reasonable sizes
        if (parsedSize < 1.0f) {
            parsedSize *= 25.0f;  // 0.27mm -> 6.75, 0.42mm -> 10.5
        }
        text.fontSize = parsedSize;

        // Parse text-anchor
        std::string anchor = GetAttr(attrs, "text-anchor");
        if (anchor == "middle") text.anchor = 1;
        else if (anchor == "end") text.anchor = 2;
        else text.anchor = 0; // start

        // Use LCARS orange for all SVG text to ensure visibility
        text.color = kOrange;

        textElements_.push_back(text);
        ++textIt;
    }
}

// Parse a transform="translate(x,y)" attribute
static bool ParseTranslate(const std::string& transform, float& tx, float& ty) {
    std::regex translateRegex(R"(translate\(\s*([+-]?\d*\.?\d+)\s*,\s*([+-]?\d*\.?\d+)\s*\))");
    std::smatch match;
    if (std::regex_search(transform, match, translateRegex)) {
        tx = SafeStof(match[1].str());
        ty = SafeStof(match[2].str());
        return true;
    }
    return false;
}

// Parse bounds from common shape elements within a group
static void UpdateBoundsFromElement(const std::string& element, float tx, float ty,
                                     float& minX, float& minY, float& maxX, float& maxY) {
    // For rect: extract x, y, width, height attributes (in any order)
    if (element.find("<rect") != std::string::npos) {
        std::string xStr = GetAttr(element, "x");
        std::string yStr = GetAttr(element, "y");
        std::string wStr = GetAttr(element, "width");
        std::string hStr = GetAttr(element, "height");
        if (!xStr.empty() && !yStr.empty() && !wStr.empty() && !hStr.empty()) {
            float x = SafeStof(xStr) + tx;
            float y = SafeStof(yStr) + ty;
            float w = SafeStof(wStr);
            float h = SafeStof(hStr);
            minX = std::min(minX, x);
            minY = std::min(minY, y);
            maxX = std::max(maxX, x + w);
            maxY = std::max(maxY, y + h);
        }
        return;
    }

    // For circle: extract cx, cy, r attributes
    if (element.find("<circle") != std::string::npos) {
        std::string cxStr = GetAttr(element, "cx");
        std::string cyStr = GetAttr(element, "cy");
        std::string rStr = GetAttr(element, "r");
        if (!cxStr.empty() && !cyStr.empty() && !rStr.empty()) {
            float cx = SafeStof(cxStr) + tx;
            float cy = SafeStof(cyStr) + ty;
            float r = SafeStof(rStr);
            minX = std::min(minX, cx - r);
            minY = std::min(minY, cy - r);
            maxX = std::max(maxX, cx + r);
            maxY = std::max(maxY, cy + r);
        }
        return;
    }

    // For line: extract x1, y1, x2, y2 attributes
    if (element.find("<line") != std::string::npos) {
        std::string x1Str = GetAttr(element, "x1");
        std::string y1Str = GetAttr(element, "y1");
        std::string x2Str = GetAttr(element, "x2");
        std::string y2Str = GetAttr(element, "y2");
        if (!x1Str.empty() && !y1Str.empty() && !x2Str.empty() && !y2Str.empty()) {
            float x1 = SafeStof(x1Str) + tx;
            float y1 = SafeStof(y1Str) + ty;
            float x2 = SafeStof(x2Str) + tx;
            float y2 = SafeStof(y2Str) + ty;
            minX = std::min(minX, std::min(x1, x2));
            minY = std::min(minY, std::min(y1, y2));
            maxX = std::max(maxX, std::max(x1, x2));
            maxY = std::max(maxY, std::max(y1, y2));
        }
        return;
    }

    // For path: extract coordinates from d attribute
    if (element.find("<path") != std::string::npos) {
        std::string d = GetAttr(element, "d");
        if (!d.empty()) {
            // Extract all numeric values (coordinates) from the path
            std::regex numRegex(R"([+-]?\d+\.?\d*)");
            std::sregex_iterator it(d.begin(), d.end(), numRegex);
            std::sregex_iterator end;
            bool isX = true;
            float lastX = 0, lastY = 0;
            while (it != end) {
                float val = SafeStof((*it)[0].str());
                if (isX) {
                    lastX = val + tx;
                    minX = std::min(minX, lastX);
                    maxX = std::max(maxX, lastX);
                } else {
                    lastY = val + ty;
                    minY = std::min(minY, lastY);
                    maxY = std::max(maxY, lastY);
                }
                isX = !isX;
                ++it;
            }
        }
    }
}

void SvgRenderer::ParseGroupElements(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Find all <g id="..." transform="translate(...)">...</g> elements
    // This regex captures: id, transform, and the entire group content
    std::regex groupRegex(R"(<g\s+id=\"([^\"]+)\"[^>]*transform=\"([^\"]+)\"[^>]*>([\s\S]*?)</g>)");
    std::smatch match;
    std::string::const_iterator searchStart(content.cbegin());

    while (std::regex_search(searchStart, content.cend(), match, groupRegex)) {
        std::string id = match[1].str();
        std::string transform = match[2].str();
        std::string groupContent = match[3].str();

        float tx = 0, ty = 0;
        ParseTranslate(transform, tx, ty);

        // Calculate bounds from child elements
        float minX = std::numeric_limits<float>::max();
        float minY = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float maxY = std::numeric_limits<float>::lowest();

        // Find all child elements (rect, circle, path, line)
        std::regex elementRegex(R"(<(rect|circle|path|line)[^>]*>)");
        std::sregex_iterator it(groupContent.begin(), groupContent.end(), elementRegex);
        std::sregex_iterator end;
        while (it != end) {
            UpdateBoundsFromElement((*it)[0].str(), tx, ty, minX, minY, maxX, maxY);
            ++it;
        }

        // Only add if we found valid bounds
        if (minX < maxX && minY < maxY) {
            SvgGroup group;
            group.id = id;
            // Convert from mm to pixels (96 DPI) to match nanosvg coordinate space
            constexpr float mmToPx = 96.0f / 25.4f;  // ~3.78 px/mm
            group.bounds[0] = (minX - viewBoxX_) * mmToPx;
            group.bounds[1] = (minY - viewBoxY_) * mmToPx;
            group.bounds[2] = (maxX - viewBoxX_) * mmToPx;
            group.bounds[3] = (maxY - viewBoxY_) * mmToPx;
            groupElements_.push_back(group);
        }

        searchStart = match.suffix().first;
    }
}

bool SvgRenderer::ShapeInGroup(const float shapeBounds[4], const SvgGroup& group) const {
    // Check if shape's center is within group bounds (with small tolerance)
    float shapeCX = (shapeBounds[0] + shapeBounds[2]) * 0.5f;
    float shapeCY = (shapeBounds[1] + shapeBounds[3]) * 0.5f;
    float tolerance = 2.0f;  // Small tolerance for floating point
    return shapeCX >= group.bounds[0] - tolerance &&
           shapeCX <= group.bounds[2] + tolerance &&
           shapeCY >= group.bounds[1] - tolerance &&
           shapeCY <= group.bounds[3] + tolerance;
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

        // First check direct shape ID override
        if (shape->id[0] != '\0') {
            auto it = colorOverrides_.find(shape->id);
            if (it != colorOverrides_.end()) {
                strokeColor = it->second;
                fillColor = it->second;
                hasColorOverride = true;
            }
        }

        // If no direct override, check if shape falls within any overridden group
        if (!hasColorOverride) {
            for (const auto& group : groupElements_) {
                auto it = colorOverrides_.find(group.id);
                if (it != colorOverrides_.end() && ShapeInGroup(shape->bounds, group)) {
                    strokeColor = it->second;
                    fillColor = it->second;
                    hasColorOverride = true;
                    break;
                }
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

    // First check nanosvg shapes (direct shape IDs)
    for (NSVGshape* shape = image_->shapes; shape != nullptr; shape = shape->next) {
        if (strcmp(shape->id, id) == 0) {
            float x = offsetX + shape->bounds[0] * scale;
            float y = offsetY + shape->bounds[1] * scale;
            float w = (shape->bounds[2] - shape->bounds[0]) * scale;
            float h = (shape->bounds[3] - shape->bounds[1]) * scale;
            return ImVec4(x, y, w, h);
        }
    }

    // Then check parsed group elements
    for (const auto& group : groupElements_) {
        if (group.id == id) {
            float x = offsetX + group.bounds[0] * scale;
            float y = offsetY + group.bounds[1] * scale;
            float w = (group.bounds[2] - group.bounds[0]) * scale;
            float h = (group.bounds[3] - group.bounds[1]) * scale;
            return ImVec4(x, y, w, h);
        }
    }

    return ImVec4(0, 0, 0, 0);
}

std::vector<std::string> SvgRenderer::GetShapeIds() const {
    std::vector<std::string> ids;
    if (!image_) return ids;

    // Add group IDs first (these are the primary component IDs)
    for (const auto& group : groupElements_) {
        ids.push_back(group.id);
    }

    // Add direct shape IDs (if any)
    for (NSVGshape* shape = image_->shapes; shape != nullptr; shape = shape->next) {
        if (shape->id[0] != '\0') {
            ids.emplace_back(shape->id);
        }
    }
    return ids;
}
