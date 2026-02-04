# S.T.A.R.S. Terminal — TUI Rendering Notes

Synchronized Transparent Access/Retrieval System

Notes on rendering the STARS-style UI using only terminal characters (ASCII,
Unicode box-drawing, block elements, braille) for a TUI version of the
interface.

## Character Palette

### Block elements (structural)

| Char | Name                  | Use                                  |
|------|-----------------------|--------------------------------------|
| `█`  | Full block            | Thick bars, sidebar, buttons         |
| `▀`  | Upper half block      | Top cap of bars                      |
| `▄`  | Lower half block      | Bottom cap of bars                   |
| `▌`  | Left half block       | Narrow vertical dividers             |
| `▐`  | Right half block      | Narrow vertical dividers             |

### Quadrant blocks (smooth elbows)

| Char | Name                        | Use                           |
|------|-----------------------------|-------------------------------|
| `▛`  | Upper-left + upper-right + lower-left | Top-left inner elbow  |
| `▙`  | Upper-left + lower-left + lower-right | Bottom-left inner elbow |
| `▜`  | Upper-left + upper-right + lower-right | Top-right inner elbow |
| `▟`  | Upper-right + lower-left + lower-right | Bottom-right inner elbow |

### Triangle blocks (45-degree elbows)

| Char | Name                    | Use                              |
|------|-------------------------|----------------------------------|
| `◤`  | Upper-left triangle     | Top-left inner chamfer           |
| `◥`  | Upper-right triangle    | Top-right inner chamfer          |
| `◣`  | Lower-left triangle     | Bottom-left inner chamfer        |
| `◢`  | Lower-right triangle    | Bottom-right inner chamfer       |

### Braille patterns (all-braille rendering)

Braille characters (U+2800–U+28FF) encode a 2×4 dot grid per cell. An
all-braille render uses no block elements at all — every structural
character is a braille pattern, giving a cohesive dot-matrix aesthetic.

```
Dot layout:     Example — ⣿ (all dots raised):
1 4             ● ●
2 5             ● ●
3 6             ● ●
7 8             ● ●
```

Block element equivalents:

| Block | Braille | Dots      | Visual                          |
|-------|---------|-----------|------------------------------- -|
| `█`   | `⣿`    | All (1–8) | Full cell — sidebar, thick bars |
| `▀`   | `⠛`    | 1,2,4,5   | Upper half — bottom cap of bars |
| `▄`   | `⣤`    | 3,6,7,8   | Lower half — top cap of bars    |
| `░`   | `⠒`    | 2,5       | Sparse fill — progress bg       |
| `▌`   | `⡇`    | 1,2,3,7   | Left column — vertical tail     |

Curve transition characters (3 cells per corner):

| Corner        | Sequence   | Dots per cell | Direction               |
|---------------|------------|---------------|-------------------------|
| Top-left      | `⡿⠿⠟`   | 7 → 6 → 5    | Full → upper, opens ↘   |
| Top-right     | `⠻⠿⢿`   | 5 → 6 → 7    | Upper → full, opens ↙   |
| Bottom-left   | `⣷⣶⣦`   | 7 → 6 → 5    | Full → lower, opens ↗   |
| Bottom-right  | `⣴⣶⣾`   | 5 → 6 → 7    | Lower → full, opens ↖   |

Each curve transitions smoothly into the horizontal line character
(`⠛` for top elbows, `⣤` for bottom elbows) — the last curve cell
and the line cell differ by only one dot.

### Box-drawing (content panels)

| Char     | Use                              |
|----------|----------------------------------|
| `┌─┐│└─┘`| Content panel borders           |
| `├┤┬┴┼`  | Panel subdivisions, tables      |
| `─ │`    | Horizontal/vertical separators   |

### Misc

| Char | Use                                        |
|------|--------------------------------------------|
| `░`  | Progress bar background / low activity      |
| `▒`  | Progress bar partial fill                   |
| `▓`  | Progress bar near-full                      |
| `·`  | Dot separators, sparse data                 |

## Color Palette (ANSI)

Map the graphical STARS palette (see DESIGN.md) to 256-color or truecolor ANSI
escape codes. The background is always black.

| Role              | Hex       | ANSI 256 approx | Description           |
|-------------------|-----------|------------------|-----------------------|
| Primary bar       | `#FF9933` | `208`            | Orange — bars, elbows |
| Secondary bar     | `#CC99CC` | `182`            | Lavender — view btns  |
| Accent bar        | `#9999FF` | `141`            | Periwinkle — segments |
| Status bar        | `#FFCC99` | `223`            | Tan — bottom bar      |
| Alert             | `#CC6666` | `167`            | Red — warnings        |
| Data values       | `#FFDDBB` | `224`            | Beige — readout text  |
| Content text      | `#FFFFFF` | `15`             | White — panel text    |
| Muted text        | `#777777` | `243`            | Gray — disabled/meta  |

## Layout Structure

The TUI layout mirrors the graphical version's panel architecture:

```
┌─────────── GLOBAL TOP BAR ──────────────────────────────────────────────┐
│ SIDEBAR  │                                                    │ SEGMENTS│
│ (view    │  TITLE ROW                                         │ (color  │
│  column) │                                                    │  coded) │
│          ├─ ELBOW ──────────────────────────────────────────────────────┤
│ [btn 1]  │                                                              │
│ [btn 2]  │  CONTENT AREA                                                │
│ [btn 3]  │                                                              │
│          ├──────────────────────────────────────────────────── ELBOW ──┤
│ SIDEBAR  │  TITLE ROW                                         │ SEGMENTS│
│          │                                                    │         │
└─────────── GLOBAL BOTTOM BAR ───────────────────────────────────────────┘
```

- **Sidebar** is drawn with `█` full blocks, 12 chars wide.
- **Buttons** alternate between a label row (`█ LABEL █`) and a spacer
  row (`████████████`) to create the pill/rectangle look.
- **Title row** and **global bars** are `▀`/`▄` capped thick lines with
  segmented gaps to mimic multi-color STARS bar sections.
- **Elbows** connect the sidebar to the title row at the inner corner.
- **Content area** uses standard box-drawing characters for sub-panels.

## Proposed Render: Smooth Elbow (▛▙)

Uses quadrant block characters for a rounded inner corner, closest to the
original STARS curve.

```
 ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 ████████████                                           █████ ████ ██████
 ████████████   S.T.A.R.S. Terminal                     █████ ████ ██████
 ████████████▛▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▜██████
 ████████████
 █  CMD-01  █  ┌─ LOCAL ───────────┐  ┌─ DISTRIBUTED ─────────┐
 ████████████  │ /usr/local  ACTIVE │  │ pg://db1       SYNC   │
 █  DAT-02  █  │ /var/data   IDLE   │  │ redis://c1     ACTIVE │
 ████████████  │ ~/docs      READY  │  │ mongo://m0     SYNC   │
 █  SCN-03  █  └────────────────────┘  └───────────────────────┘
 ████████████
 █  COM-04  █  ┌─ ACTIVITY LOG ───────────────────────────────┐
 ████████████  │ > SELECT * FROM nodes WHERE status='active'  │
 █  LOG-05  █  │   3 results (12ms)                           │
 ████████████  │ > SYNC ALL                                   │
 █  SYS-06  █  │   synchronizing 3 nodes...              ░░░  │
 ████████████  └──────────────────────────────────────────────┘
 ████████████
 ████████████▙▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▟██████
 ████████████  NODES: 5  ████  STATUS: OK  ████  SD 2026.032   ██████
 ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
```

**Elbow detail (top-left, smooth):**

```
 ████████████████████████████  ← horizontal bar
 ████████████▛▀▀▀▀▀▀▀▀▀▀▀▀▀  ← ▛ fills UL+UR+LL quadrants, leaving
 ████████████                     lower-right empty → smooth curve effect
```

`▛` reads as: the upper portion connects to the bar above, the left portion
connects to the sidebar below, and the missing lower-right quadrant creates
a concave curve into the content area.

## Proposed Render: 45-Degree Chamfer (◤◣)

Uses triangle block characters for an angular chamfered corner. Sharper,
more geometric feel — still distinctly STARS but with a harder edge.

```
 ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 ████████████                                           █████ ████ ██████
 ████████████   S.T.A.R.S. Terminal                     █████ ████ ██████
 ████████████◤▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀◥██████
 ████████████
 █  CMD-01  █  ┌─ LOCAL ───────────┐  ┌─ DISTRIBUTED ─────────┐
 ████████████  │ /usr/local  ACTIVE │  │ pg://db1       SYNC   │
 █  DAT-02  █  │ /var/data   IDLE   │  │ redis://c1     ACTIVE │
 ████████████  │ ~/docs      READY  │  │ mongo://m0     SYNC   │
 █  SCN-03  █  └────────────────────┘  └───────────────────────┘
 ████████████
 █  COM-04  █  ┌─ ACTIVITY LOG ───────────────────────────────┐
 ████████████  │ > SELECT * FROM nodes WHERE status='active'  │
 █  LOG-05  █  │   3 results (12ms)                           │
 ████████████  │ > SYNC ALL                                   │
 █  SYS-06  █  │   synchronizing 3 nodes...              ░░░  │
 ████████████  └──────────────────────────────────────────────┘
 ████████████
 ████████████◣▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄◢██████
 ████████████  NODES: 5  ████  STATUS: OK  ████  SD 2026.032   ██████
 ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
```

**Elbow detail (top-left, 45-degree):**

```
 ████████████████████████████  ← horizontal bar
 ████████████◤▀▀▀▀▀▀▀▀▀▀▀▀▀  ← ◤ fills upper-left triangle only,
 ████████████                     diagonal cuts from top-right to
                                  bottom-left → 45° chamfer
```

`◤` reads as: the upper-left triangle connects to the bar above and the
sidebar to the left, while the empty lower-right triangle creates a hard
diagonal cut into the content area.

**Multi-row chamfer variant** (larger radius, more dramatic 45° bevel):

```
 ████████████████████████████████████████████████████████████████████
 ██████████████████████████████████████████████████████████████████
 █████████████◤▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 ████████████◤
 ████████████
```

Each row steps one character inward, producing a wider-angle chamfer.
This is more visually dramatic but costs vertical space.

## Proposed Render: Braille Curve (⡿⣷)

All-braille render — uses no block elements. Every structural character
is a braille pattern. `⣿` replaces `█`, `⠛` replaces `▀`, `⣤` replaces
`▄`, and curves use 3-cell transitions that blend smoothly into the
horizontal line characters.

```
 ⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿                                           ⣿⣿⣿⣿⣿ ⣿⣿⣿⣿ ⣿⣿⣿⣿⣿⣿
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿   S.T.A.R.S. Terminal                     ⣿⣿⣿⣿⣿ ⣿⣿⣿⣿ ⣿⣿⣿⣿⣿⣿
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠿⠟⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠻⠿⢿⣿⣿⣿⣿⣿⣿
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
 ⣿  CMD-01  ⣿  ┌─ LOCAL ────────────┐  ┌─ DISTRIBUTED ─────────┐
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿  │ /usr/local  ACTIVE │  │ pg://db1       SYNC   │
 ⣿  DAT-02  ⣿  │ /var/data   IDLE   │  │ redis://c1     ACTIVE │
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿  │ ~/docs      READY  │  │ mongo://m0     SYNC   │
 ⣿  SCN-03  ⣿  └────────────────────┘  └───────────────────────┘
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
 ⣿  COM-04  ⣿  ┌─ ACTIVITY LOG ────────────────────────────────┐
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿  │ > SELECT * FROM nodes WHERE status='active'   │
 ⣿  LOG-05  ⣿  │   3 results (12ms)                            │
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿  │ > SYNC ALL                                    │
 ⣿  SYS-06  ⣿  │   synchronizing 3 nodes...              ⠒⠒⠒   │
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿  └───────────────────────────────────────────────┘
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣶⣦⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣴⣶⣾⣿⣿⣿⣿⣿⣿
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿  NODES: 5  ⣿⣿⣿⣿  STATUS: OK  ⣿⣿⣿⣿  SD 2026.032   ⣿⣿⣿⣿⣿⣿
 ⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛
```

**Elbow detail (top-left, braille):**

Three braille cells trace a smooth curve. Each step removes one dot from
the lower edge, transitioning from full-cell density down to the
upper-half horizontal line:

```
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿  ← horizontal bar (⣿ = all 8 dots)
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠿⠟⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛  ← curve: ⡿→⠿→⠟ then ⠛ line
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿                  ← sidebar continues (⣿)
```

Dot-level breakdown of the 3-cell curve into `⠛` line:

```
 ⡿         ⠿         ⠟         ⠛ (line)
 ● ●       ● ●       ● ●       ● ●
 ● ●       ● ●       ● ●       ● ●
 ● ●       ● ●       ●
 ●
```

Each cell removes one dot from the bottom of the previous cell:
`⣿`(8) → `⡿`(7) → `⠿`(6) → `⠟`(5) → `⠛`(4). The last curve
character (`⠟`) and the line character (`⠛`) differ by a single dot,
so the transition from curve to straight line is seamless.

**Multi-row braille variant** (larger radius, even smoother arc):

The curve can span 2 rows for a wider-radius arc. The first row uses
a longer sequence of transition characters, and a second row carries
the tail of the curve down into the sidebar:

```
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠿⠟⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇
 ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
```

The wider sidebar (13 `⣿` in the curve row) plus a `⡇` (left-column
dots only) tail on the next row produces a larger-radius sweep. `⡇`
reads as a thin vertical connector — the outer edge of the curve
continuing down while the inner edge has already ended.

## Comparison: Smooth vs. 45-Degree vs. Braille

| Aspect            | Smooth (`▛▙`)              | 45-Degree (`◤◣`)            | Braille (`⡿⣷`)                   |
|-------------------|----------------------------|-----------------------------|------------------------------------|
| Feel              | Organic, faithful to STARS | Geometric, modernized       | Dot-matrix / retro-terminal        |
| Block elements    | Yes (`█▀▄`)                | Yes (`█▀▄`)                 | None — all braille patterns        |
| Terminal support  | Needs quadrant blocks      | Needs geometric shapes      | Needs braille (U+2800–U+28FF)     |
| Curve fidelity    | Good (2×2 sub-cell)        | Stylized reinterpretation   | Best (2×4 dots per cell)           |
| Font sensitivity  | Moderate                   | Higher — triangles vary     | Highest — entire UI is dots        |
| Chars per corner  | 1                          | 1                           | 3 (or more for larger radius)      |
| Visual density    | Solid fills                | Solid fills                 | Uniform dot density, cohesive look |
| Fallback          | `┌┐└┘` box corners         | `/\` ASCII diagonals        | Smooth (`▛▙`) or 45° (`◤◣`)       |

All three approaches should be implemented with a fallback chain that
degrades gracefully: braille → quadrant blocks → box-drawing corners →
ASCII (`+--+||+--+`).

## Sidebar Button Construction

Buttons in the sidebar are built from alternating label and spacer rows:

```
 ████████████  ← spacer (full blocks, same width as sidebar)
 █  CMD-01  █  ← label  (block + space + text + space + block)
 ████████████  ← spacer
 █  DAT-02  █  ← label
 ████████████  ← spacer
```

- Spacer rows create visual separation (the gap between STARS buttons).
- Label rows show the button text centered between block-character walls.
- The selected/active button can be highlighted by using a different color
  or inverting the text.

## Segmented Bar Construction

Top and bottom bars are segmented with gaps to mimic multi-color STARS bars:

```
 ████████████   S.T.A.R.S. Terminal                     █████ ████ ██████
                                                        ──┬── ──┬── ──┬──
                                                      seg 1  seg 2  seg 3
```

Each segment would be a different color in the palette (orange, lavender,
periwinkle). The gaps are 1 character of black/background.

## Mapping to Graphical STARS Concepts

| Graphical (DESIGN.md)   | TUI Equivalent                              |
|-------------------------|---------------------------------------------|
| Panel                   | Full-width region bounded by elbow rows     |
| Title row               | `▀`/`▄` capped bar with elbow + title text  |
| Elbow                   | `▛`/`◤`/`⡿` corner connecting sidebar↔bar  |
| View column (sidebar)   | `█` block column, 12 chars wide             |
| View buttons            | Alternating label/spacer rows in sidebar    |
| Option column           | Could mirror on right side or omit in TUI   |
| Content area            | Box-drawn panels in remaining space         |
| Global top bar          | First `▄` row + title bar                   |
| Global bottom bar       | Status bar + final `▀` row                  |
| Gap between panels      | 1 blank row between elbow rows              |

## Implementation Notes

- Minimum terminal size: 80x24 (standard). Content panels reflow or
  truncate below this. Ideal width: 100+ columns.
- Use truecolor (`\e[38;2;R;G;Bm`) when `$COLORTERM=truecolor`, fall
  back to 256-color (`\e[38;5;Nm`), then to basic 16 colors.
- Detect Unicode support via `$LANG`/`$LC_ALL` containing `UTF-8`.
  Fall back to ASCII box-drawing (`+--+||+--+`) if not available.
- Elbow character selection (smooth vs. 45-degree vs. braille) could be
  a user preference or auto-detected based on terminal/font capabilities.
- The all-braille render looks best in fonts where braille dots are
  rendered as small filled circles (e.g., JetBrains Mono, Iosevka).
  Since the entire UI is braille, there are no density mismatches
  between dots and solid blocks — but dot size and spacing still vary
  significantly across fonts and terminals.
