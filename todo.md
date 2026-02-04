# Next Steps: Real GOES Data in the UI

Now that `data/cache/goes_orbital.json` and `data/cache/goes_mag.json` feed real satellite data into the seed database, these are recommended next steps for surfacing that data more effectively in the STARS UI.

## 1. Update RADIO field labels to reflect real units

The RADIO view fields (45-50) in `field_defs.h` still use labels like "RANGE" with implied fictional units. When real orbital data is present, the values are real-world quantities (km, dBm, degrees). Update the field definitions to match:

- **Field 45**: Label could show "GOES-16 LON" or "SUB-SAT POINT" instead of generic frequency
- **Field 50**: Change units hint from "LY" to "km" when orbital cache is active

This could be done statically in `field_defs.h`, or dynamically by having `gen_graphs.py` write a small metadata row the UI can check at startup.

**Files**: `src/field_defs.h`

## 2. Add a GOES orbital map to the SHIP view

The SHIP view currently renders `assets/ship_wireframe.svg` with data-bound overlays via `SvgRenderer`. A second SVG showing Earth with the GOES-16 sub-satellite point and ground station (Wallops Island) would make the orbital data tangible. The existing `SvgBindingDef` system can drive color/label overlays on the map shapes.

- Create `assets/goes_orbit.svg` with shapes for: Earth outline, GOES-16 marker, ground station marker, line-of-sight arc
- Add SVG bindings in `field_defs.h` linking orbital fields to shape colors (e.g., signal strength drives link color: green/yellow/red)
- Wire it into a new view or an option group toggle on the existing SHIP view

**Files**: `assets/goes_orbit.svg` (new), `src/field_defs.h`, `src/tactical_panel.cpp`

## 3. Make graph 0 title context-aware

Graph 0 is labeled "SENSOR TELEMETRY" in `kGraphs[]` (`field_defs.h`). When it contains real magnetometer data, the axis labels and title should reflect that — e.g., "MAG FIELD (nT)" for the Y axis, "Bx" / "By" for the line names. This could be:

- A build-time change (just update the label since we always intend to use real mag data going forward)
- A runtime check: `DbReader` could look for a metadata flag in the DB and swap the graph title

**Files**: `src/field_defs.h`, possibly `src/db_reader.cpp`

## 4. Add time-varying orbital scalars

Currently the orbital data maps to "constant" fields (45-50) that show the same value type at every timestamp but with varying values from the propagated orbit. The UI renders these as static label/value pairs. To make the orbital variation visible:

- Add a new graph (graph ID 4) plotting altitude and range over the 100 timestamps
- Define it in `kGraphs[]` with view "RADIO", Y-axis range appropriate for GEO altitude (~35,750-35,820 km)
- `gen_graphs.py` already writes per-timestamp orbital values; a second pass could emit graph_data rows from the orbital cache

**Files**: `src/field_defs.h`, `data/gen_graphs.py`, `data/seed.sql` (if graph metadata table exists)

## 5. Color-code RADIO fields by threshold

The `SvgBindingDef` system already supports `warnThresh` / `critThresh` with directional coloring (`LowIsWorse` / `HighIsWorse`). The same logic (`EvalFieldColor` in `field_renderer.cpp`) could be applied to scalar field rendering in the RADIO view:

- Signal strength (field 46): warn below -50 dBm, critical below -60 dBm
- SNR (field 48): warn below 60 dB, critical below 50 dB
- Range (field 50): informational only, no threshold

This would require extending `DrawFieldColumn()` to optionally apply color based on the numeric_value and per-field thresholds.

**Files**: `src/field_renderer.cpp`, `src/field_defs.h`

## 6. Live data refresh (longer term)

The current pipeline is offline: `make fetch` downloads once, `make reseed` rebuilds the DB. For live updates:

- Add a background thread or timer in `main.cpp` that re-fetches the TLE every few hours (CelesTrak updates daily)
- Re-propagate the orbit from the fresh TLE and write new orbital rows
- `DbReader` already cycles through timestamps; extending it to append new data or swap DB files would enable near-real-time orbital tracking
- The MAG data could be refreshed from the rolling S3 archive on a similar cadence

This is a larger change touching `src/db_reader.cpp`, `src/main.cpp`, and potentially adding a new `src/data_fetcher.*` module.

## 7. Add GOES-18 / multi-satellite support

The fetch pipeline currently targets GOES-16 only. The CelesTrak JSON contains all 20 GOES satellites. Extending the fetcher to propagate GOES-18 (GOES-West) alongside GOES-16 (GOES-East) would allow:

- A comparison view showing both satellites
- Toggle between satellites via the RADIO view's option group buttons ("GOES-E" / "GOES-W")
- Different field IDs or a satellite selector that swaps which orbital data populates fields 45-50

**Files**: `data/fetch_tle.py`, `data/gen_graphs.py`, `src/field_defs.h`, `src/tactical_panel.cpp`

## Priority Order

| # | Item | Scope | Touches C++ |
|---|------|-------|-------------|
| 1 | Update RADIO field labels | Small | Yes |
| 3 | Context-aware graph title | Small | Yes |
| 5 | Color-code RADIO by threshold | Medium | Yes |
| 4 | Orbital altitude/range graph | Medium | Yes + Python |
| 2 | GOES orbital map SVG | Medium | Yes + new asset |
| 7 | Multi-satellite support | Medium | Yes + Python |
| 6 | Live data refresh | Large | Yes + new module |
