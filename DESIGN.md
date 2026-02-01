# LCARS UI Design Rules

## Architecture

```
src/
  main.cpp              App bootstrap (SDL, OpenGL, ImGui), main loop, screen composition
  lcars.h               Shared header: color palette, theme, panel types, drawing helpers
  lcars.cpp             Color definitions, ApplyLCARSTheme(), global chrome, panel chrome drawing
  tactical_panel.h      TacticalPanel declaration
  tactical_panel.cpp    TacticalPanel content (system status + telemetry chart)
```

## Core Concepts

### Screen

A screen is the full application window. It contains:
- A **global top bar** and **global bottom bar** (app-level chrome).
- One or more **panels** stacked vertically between the global bars.

### Panel

A panel is a self-contained rectangular region. Each panel owns:
- A **title row** (horizontal bar with the panel title and an elbow).
- A **view column** (vertical strip of rectangular buttons for switching views).
- An **option column** (vertical strip of pill-shaped buttons for configuration).
- A **content area** (the remaining rectangle where the view draws its widgets).
- An **elbow** (the curved corner piece connecting the title row to the view
  column, displaying the name of the currently active view).

Not all elements must be present -- a panel may have zero view buttons and/or
zero option buttons. Even when empty, the view and option columns still reserve
their space to keep layout consistent across panels.

### View

A view is one mode of a panel. The panel's view buttons let the user switch
between them. Each view provides its own content drawing function and may define
its own set of option buttons. The elbow text updates to show the currently
selected view's name.

### Option

Options are configuration buttons within a view. They are pill-shaped and appear
in the option column. Options may be organized into groups separated by thin
horizontal dividers.

## Panel Orientation

Each panel has two orientation axes:

### Horizontal: left or right (default: left)

Controls which side is the **view column** and which is the **option column**.

| Orientation | View column | Option column |
|-------------|-------------|---------------|
| Left        | Left side   | Right side    |
| Right       | Right side  | Left side     |

### Vertical: top or bottom

Controls where the **title row** is placed.

| Orientation | Title row position |
|-------------|--------------------|
| Top         | Top edge of panel  |
| Bottom      | Bottom edge of panel |

### Composition rule

When multiple panels are stacked on a screen:
- The **top-most panel** is always **bottom** orientation (title row at its
  bottom edge).
- All **subsequent panels** are **top** orientation (title row at their top
  edge).

This means adjacent panels have their title rows next to each other, forming a
natural visual divider. Each panel draws its own title bar independently -- they
do not merge. There is a gap between them.

## Panel Layout

### Title row

A horizontal bar spanning the panel width. Contains:
- The **elbow button** at the end closest to the view column.
- The **panel title text** at the opposite end from the elbow.

The title text is rendered in black on the bar's surface.

### Elbow

The elbow is the curved corner piece where the title row meets the view column.
It is drawn as a filled rectangle covering the corner, with a black quarter-
circle arc cut out of the inner corner (the signature LCARS curve). The elbow:
- Occupies the corner of the title row and extends into the view column.
- Displays text showing the **currently selected view name** (black text).
- Is **not clickable** (decorative/informational only).
- Curves **away from the content area**: a top title row's elbow curves upward
  into the view column; a bottom title row's elbow curves downward.

### View buttons

Rectangular buttons (sharp corners, no rounding) stacked vertically in the view
column. They represent different modes/views of the panel. Clicking a view
button switches the panel's active view, which:
- Changes the content area rendering.
- Updates the elbow text to the new view name.
- May change the option buttons to those defined by the new view.

### Option buttons

Pill-shaped buttons (fully rounded ends) stacked vertically in the option
column. They represent configuration or actions for the current view. Options
may be divided into **groups** separated by thin horizontal dividers.

### Content area

The remaining rectangle after the title row, view column, and option column are
subtracted. An ImGui window is created here by the main loop; the panel's active
view draws its widgets inside it.

## Visual Layout Diagram

Two-panel screen with default left orientation:

```
+========================= GLOBAL TOP BAR ==========================+
|              |                                        |            |
| [view btn]   |                                        | [opt btn]  |
| [view btn]   |       PANEL 1 CONTENT                  | [opt btn]  |
|              |                                        |            |
| [ELBOW]======[======= PANEL 1 TITLE ====== "Title 1"]=============+
|              gap                                                   |
| [ELBOW]======[======= PANEL 2 TITLE ====== "Title 2"]=============+
|              |                                        |            |
| [view btn]   |       PANEL 2 CONTENT                  | [opt btn]  |
| [view btn]   |                                        | [opt btn]  |
|              |                                        |            |
+========================= GLOBAL BOTTOM BAR ========================+
```

Panel 1 is `bottom` orientation: title row at its bottom, elbow curves upward
from the title row into the view column above.

Panel 2 is `top` orientation: title row at its top, elbow curves downward from
the title row into the view column below.

## Color Palette

| Name   | Hex       | ImU32 constant | Usage                                    |
|--------|-----------|----------------|------------------------------------------|
| Orange | `#FF9933` | `kOrange`      | Title rows, elbows, global bars, primary |
| Purple | `#CC99CC` | `kPurple`      | View buttons, labels                     |
| Blue   | `#9999FF` | `kBlue`        | View buttons, secondary plot lines       |
| Tan    | `#FFCC99` | `kTan`         | Global bottom bar, section headings      |
| Red    | `#CC6666` | `kRed`         | Reserved (alerts/warnings)               |
| Beige  | `#FFDDBB` | `kBeige`       | Data readout values                      |

Background is pure black (`0, 0, 0`). Window backgrounds are near-black
(`0.02, 0.02, 0.04`). Default text color is orange.

Use `U32ToVec4(ImU32)` to convert palette constants to `ImVec4` for ImGui color
functions like `ImGui::TextColored()` and `ImGui::PushStyleColor()`.

## Global Chrome

The global top bar and bottom bar are drawn by the app infrastructure, not by
panels. They are thin horizontal bars at the very top and bottom edges of the
window.

| Element          | Color    | Height |
|------------------|----------|--------|
| Global top bar   | `kOrange`| 40px   |
| Global bottom bar| `kTan`   | 30px   |

The global top bar has a 20px rounded right end. The global bottom bar has a
20px rounded left end (starting after a gap from the left edge).

## Fixed Dimensions

| Element                   | Value (px) |
|---------------------------|------------|
| Global top bar height     | 40         |
| Global bottom bar height  | 30         |
| View column width         | 120        |
| Option column width       | 120        |
| Title row height          | 30         |
| Elbow radius (main)       | 80         |
| Elbow radius (divider)    | 40         |
| Gap between elements      | 6          |
| Gap between panels        | 10         |
| View button corner radius | 0 (sharp)  |
| Option button corner radius | fully rounded (pill) |

## Elbow Construction

The elbow is drawn as a filled rectangle covering the corner where the title row
meets the view column, then a black convex polygon (quarter-circle arc + center
point) is drawn on top to carve out the inner curve. The arc uses 32 segments.

The arc sweep direction depends on orientation:

| Title position | View side | Arc sweeps         | Visual curve direction |
|----------------|-----------|--------------------|------------------------|
| Bottom         | Left      | PI to 1.5*PI       | Curves upward-left     |
| Top            | Left      | 0 to PI/2          | Curves downward-left   |
| Bottom         | Right     | 1.5*PI to 2*PI     | Curves upward-right    |
| Top            | Right     | PI/2 to PI         | Curves downward-right  |

## Text Conventions

- **Panel title**: Black text on the title row bar, positioned opposite the
  elbow.
- **Elbow text**: Black text on the elbow, showing the active view name.
- **Section headings**: `ImGui::SeparatorText()` with text color `kTan`.
- **Labels** (left column): `kPurple` via `ImGui::TextColored()`.
- **Values** (right column): `kBeige` via `ImGui::TextColored()`.
- **Key-value layouts**: `ImGui::Columns(2, nullptr, false)` with first column
  width 200px.

## ImPlot Conventions

- Plot background: near-black (`0.02, 0.02, 0.06`)
- Plot border: dim amber (`0.30, 0.25, 0.15, 0.50`)
- Axis text: `kTan`
- Axis grid: dim (`0.15, 0.12, 0.10, 0.40`)
- Legend background: near-black with 90% opacity
- Legend text: `kOrange`
- Line weight: 2px
- Plot padding: 12px
- Primary data line: `kOrange`
- Secondary data line: `kBlue`
- Plots fill available space via `ImGui::GetContentRegionAvail()`
- Minimum plot height: 100px

## ImGui Theme

| Property            | Value       |
|---------------------|-------------|
| WindowRounding      | 0           |
| FrameRounding       | 8           |
| GrabRounding        | 4           |
| ScrollbarRounding   | 4           |
| WindowBorderSize    | 0           |
| FrameBorderSize     | 0           |
| WindowPadding       | (12, 12)    |
| FramePadding        | (8, 4)      |
| ItemSpacing         | (8, 6)      |

Interactive elements follow this pattern:
- **Normal**: dark purple-tinted background
- **Hovered**: `kPurple`
- **Active/Pressed**: `kOrange`

Scrollbar grabs: purple -> blue (hover) -> orange (active).

## Adding a New Panel

1. Create `src/my_panel.h` and `src/my_panel.cpp`.
2. Subclass `LCARSPanel`.
3. Define the panel's title, views, and options per view.
4. Each view provides a content drawing function and optionally a list of option
   groups with pill-shaped buttons.
5. Add `src/my_panel.cpp` to the `add_executable` list in `CMakeLists.txt`.
6. In `main.cpp`, include the header and add it to the screen's panel list.
7. Use only the `kColor` constants and `U32ToVec4()` from `lcars.h` for colors.
8. Use `ImGui::GetContentRegionAvail()` for sizing flexible content.
9. Do not create your own `ImGui::Begin()` / `ImGui::End()` calls -- the main
   loop handles window management.
