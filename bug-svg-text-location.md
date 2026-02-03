# Bug: SVG Text Location Incorrect

## Problem
Text elements parsed from SVG files appear in the wrong location (top-left corner) instead of near their associated components.

## Context
- nanosvg library doesn't support `<text>` elements, so we added custom text parsing
- Text parsing extracts x, y, font-size, text-anchor, and color from `<text>` elements
- The RF circuit SVG (`assets/rf_circuit.svg`) has a cropped viewBox: `viewBox="37 64 153 79"`

## What Was Tried

### 1. Basic text parsing (commit 18ad388)
- Added regex parsing for `<text>` elements in `ParseTextElements()`
- Rendered text using ImGui's `AddText()` with the same coordinate transform as paths
- **Result**: Text appeared in top-left, not near components

### 2. ViewBox offset compensation (commit 238f0e7)
- Parsed the viewBox attribute to extract minX, minY offset
- Subtracted viewBox offset from text coordinates before storing
- Added `viewBoxX_` and `viewBoxY_` member variables
- **Result**: Still didn't work - text still in wrong location

## Suspected Issues

1. **Coordinate system mismatch**: nanosvg may apply additional transforms beyond just the viewBox offset. It converts coordinates to pixels at 96 DPI and may have internal scaling.

2. **Transform inheritance**: Some `<text>` elements in the RF SVG are inside `<g>` elements with `transform` attributes (e.g., `transform="rotate(-90 77.47 116.08)"`). These transforms are not being parsed.

3. **nanosvg internal transform**: nanosvg's `image_->width` and `image_->height` reflect the viewBox dimensions, but the actual coordinate transform it applies to paths may be more complex.

## Files Modified
- `src/svg_renderer.h` - Added `SvgText` struct, `textElements_` vector, `viewBoxX_/Y_` members
- `src/svg_renderer.cpp` - Added `ParseTextElements()`, viewBox parsing, text rendering in `Draw()`

## Potential Solutions to Investigate

1. **Debug coordinate values**: Print out the parsed text coordinates and the transform being applied to see where the mismatch occurs.

2. **Parse group transforms**: Handle `<g transform="...">` and apply them to nested text elements.

3. **Use nanosvg's coordinate system**: Instead of parsing text separately, see if nanosvg exposes its internal transform or if we can compute it from `image_->width/height` vs the original SVG dimensions.

4. **Compare with path coordinates**: Find a path element near a text element and compare their raw SVG coordinates vs their rendered positions.

5. **Alternative approach**: Instead of parsing SVG text, use the existing `SvgBindingDef` system to overlay ImGui text labels anchored to shape IDs (like we do for SHIP view).

## Workaround
Use `SvgBindingDef` system to place data-bound labels near component shapes by ID, similar to how the SHIP view displays shield/hull/warp core values. This doesn't show static SVG text but allows dynamic data display.
