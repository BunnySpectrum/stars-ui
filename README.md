# STARS: System Telemetry Access Retrieval System

A sci-fi themed telemetry dashboard inspired by LCARS, built with ImGui/ImPlot and OpenGL.

## POC Version (`poc/`)

The proof-of-concept version features:
- Multi-panel layout with LCARS-style chrome (elbows, view buttons, option columns)
- Six views: Tactical, Ship, Environ, Power, Radio, Map
- Animated SVG overlays with data bindings (ship wireframe, orbital map, RF circuit)
- Real-time graphs using ImPlot
- SQLite database polling for live telemetry updates
- Custom fonts (Antonio, Aurebesh)

### Build & Run

```bash
bazel run //poc/src:stars -- $(pwd)/poc/data/stars.db
```
