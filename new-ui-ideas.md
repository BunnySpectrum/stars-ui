Visualization options:
- bar gauge: horizontal or vertical bar gauges with thresholds for displaying single values with context
- stat panels: simple display of latest values with optional sparkline/trend indicators
- state timeline: for tracking state changes over time (on/off, modes, etc)
- XY charts: for plotting relationships between variables (especially seen in navigation/positioning data)
- regular tables: for displaying structured data in tabular format
- graphs: traditional graph panels with multiple series and customizable axes
- text/markdown panels: for documentation and contextual information
- gauges: radial gauges for displaying values against thresholds
- heatmaps: for visualizing matrices of data, especially temperature distributions
- LED indicators: boolean status displays (on/off) in compact format

Units: support at least the following
- electrical: volt, ampere, watt, dBm, dB
- temperature: celsius, fahereheit, kelvin
- angular: degree, rothz (rotation in hertz)
- time: seconds, microseconds, milliseconds, minutes, hours
- rate: Hz, GHz, rotghz, counts per seconds, C/min (temperature rate)

Allow for custom unit definitions.

Layout
- grid system: flexible grid positioning system (h/w/x/y coordinates)
- multi-column layout: support for arranging panels side-by-side

Data filtering & selection
- time range selection: dynamic time window adjustment with presets and custom ranges
- series toggling: ability to show/hide series in visualizations
- filtering: filtering data by various dimensions within panels

Controls
- data zoom: ability to zoom into specific time ranges or data subsets
- tooltips: tick tooltip support showing multiple values at cursor position
- click-through navigation: links between views or external resources
- annotations: support for marking events or regions on time series
- thresholds: visual indicators for when values exceed specified ranges

Misc
- refresh: configurable refresh rates for real-time data
- data exploration: ad-hox querying and data investifation capabilities
- shared crosshair: synchronized cursor position across multiple panels
- branding options: ability to customize non-functional look and feel (logos in corner, UI element styles)
- theme support: light, dark, custom


Data
- statistical functions: min, max, avg, sum, stdev, count, last, etc
- time-based aggregation: group data by time intervals
- calculated fields: create new metris based on existing ones
- transformations: join, pivot, organize data within the visualization layer

Accessibility
- color sets: ability to swap out the colors to support different themes (e.g. types of color blindness)
- keyboard navigation: full keyboard control 
- screen reader support: accessible visualizations


