# SVG Export Feedback for STARS UI Integration

This document describes requirements for KiCad-exported SVGs to be compatible with our STARS UI system.

## Summary of Requirements

1. Add `id` attributes to shapes that should be interactive
2. Use presentation attributes instead of inline `style` blocks
3. Keep text as `<text>` elements (not stroked paths)
4. **NEW: Use shapes with area for data label anchoring (not line-only paths)**

---

## 1. Add `id` Attributes to Shapes

Every shape that should change color or have a label overlay needs a unique `id`:

```xml
<!-- Good - shape can be targeted by id -->
<rect id="mixer-u2" x="83.82" y="83.82" width="12.7" height="12.7" />

<!-- Bad - no way to target this shape -->
<rect x="83.82" y="83.82" width="12.7" height="12.7" />
```

### Suggested ID Naming Convention

| Component | Suggested ID |
|-----------|--------------|
| Resistor R1 | `resistor-r1` |
| Capacitor C3 | `capacitor-c3` |
| IC U4 | `ic-u4` |
| Mixer U2 | `mixer-u2` |
| Oscillator Y1 | `oscillator-y1` |
| DAC U1 | `dac-u1` |
| Antenna AE1 | `antenna-ae1` |

---

## 2. Use Presentation Attributes Instead of Inline Styles

```xml
<!-- Good - presentation attributes -->
<path fill="none" stroke="#840000" stroke-width="0.1524" d="..." />

<!-- Bad - inline style -->
<path style="fill:none; stroke:#840000; stroke-width:0.1524;" d="..." />
```

---

## 3. Keep Text as Text Elements

```xml
<!-- Good - actual text element -->
<text x="77.47" y="137.79" font-size="10" fill="#006464">GND</text>

<!-- Bad - text converted to stroked paths -->
<g class="stroked-text"><desc>GND</desc>
  <path d="M76.50 136.53 L76.38 136.47" />
  ...
</g>
```

---

## 4. Use Shapes with Area for Data Label Anchoring

**This is important for components that will have data labels attached.**

The STARS UI anchors data labels to shapes by finding the shape's bounding box. **Line-only paths have zero width or height, so labels cannot be anchored to them.**

### Problem Example

```xml
<!-- Bad for label anchoring - this is just a vertical line with no width -->
<path id="antenna-ae1" d="m179.07 83.82v6.35" stroke="#840000" />
```

The UI calculates bounds as `{minX, minY, width, height}`. For this vertical line:
- width = 0 (line has no horizontal extent)
- **Label anchoring fails because width <= 0**

### Solution

For components that need data labels, use shapes with actual area:

```xml
<!-- Good - rectangle has width and height -->
<rect id="antenna-ae1" x="177" y="82" width="4" height="10" fill="none" stroke="#840000" />

<!-- Good - circle has diameter -->
<circle id="antenna-ae1" cx="179" cy="87" r="3" fill="none" stroke="#840000" />

<!-- Good - closed path with area -->
<path id="antenna-ae1" d="M177,82 h4 v10 h-4 z" fill="none" stroke="#840000" />
```

### Which Shapes Work

| Shape Type | Works for Labels | Notes |
|------------|------------------|-------|
| `<rect>` | Yes | Has explicit width/height |
| `<circle>` | Yes | Bounds = diameter × diameter |
| `<ellipse>` | Yes | Bounds from rx/ry |
| Closed `<path>` | Yes | If encloses area |
| Line `<path>` | **No** | Zero width or height |
| `<line>` | **No** | Zero width or height |
| `<polyline>` | **No** | Unless forms closed area |

### Recommendation

For components like antennas, connectors, or test points that are typically drawn as lines, add a small invisible rectangle or circle with the `id` for label anchoring:

```xml
<!-- Visible antenna line (no id needed) -->
<path d="m179.07 83.82v6.35" stroke="#840000" stroke-width=".254" />

<!-- Invisible anchor rectangle for label binding -->
<rect id="antenna-ae1" x="176" y="81" width="6" height="12"
      fill="none" stroke="none" />
```

Or make the antenna a closed shape:

```xml
<!-- Antenna as a small rectangle instead of a line -->
<rect id="antenna-ae1" x="178" y="83" width="2" height="8"
      fill="none" stroke="#840000" stroke-width=".254" />
```

---

## 5. Testing Your Export

Run these checks on your exported SVG:

```bash
# Count IDs (should be > 0)
grep -c 'id="' your-file.svg

# Check for inline styles (should be 0)
grep -c 'style="' your-file.svg

# Check for stroked-text (should be 0)
grep -c 'stroked-text' your-file.svg

# List all shape IDs
grep -o 'id="[^"]*"' your-file.svg | sort | uniq
```

You can also use our analyzer script:

```bash
python3 scripts/analyze_svg.py your-file.svg
```

---

## 6. Color Palette (Optional)

To match our UI theme:

| Purpose | Hex Code |
|---------|----------|
| Primary lines | `#9999FF` |
| Accent/highlight | `#FF9933` |
| Secondary | `#CC99CC` |
| Warning | `#FFCC99` |
| Critical | `#CC6666` |
| Background detail | `#6688AA` |
