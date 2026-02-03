# Mock Data Sources for GEO Satellite Model

## Overview

This document captures free, publicly available data sources from NASA and related agencies that can be used as placeholder/mock data for a geostationary (GEO) satellite model. The primary recommendation is to use GOES-R series data, as it provides both orbital elements and real spacecraft telemetry.

## Orbital Data (TLE)

Two-Line Element sets encode orbital parameters for Earth-orbiting satellites and can be used to compute position and velocity at any point in time via SGP4/SDP4 propagation.

### CelesTrak (Recommended)

- **Active GEO satellites (table):** https://celestrak.org/norad/elements/table-geo.php
- **Active GEO satellites (raw TLE):** https://celestrak.org/NORAD/elements/gp.php?GROUP=geo&FORMAT=tle
- **Active GEO satellites (JSON):** https://celestrak.org/NORAD/elements/gp.php?GROUP=geo&FORMAT=json
- **GOES-specific group:** https://celestrak.org/norad/elements/table.php?GROUP=goes&FORMAT=tle

No authentication required. Data updated daily. For GEO satellites, refreshing every 3+ days is sufficient since their orbits are very stable. GEO TLEs will show ~0 deg inclination and ~1 rev/day mean motion.

### NASA TLE API

- **Endpoint:** https://api.nasa.gov (search for TLE API)
- **Backend source:** https://github.com/ivanstan/tle-api
- **Frontend:** https://tle.ivanstanojevic.me

REST API returning TLEs in JSON format. Sourced from CelesTrak daily. Free API key from api.nasa.gov.

### Space-Track.org

- **URL:** https://www.space-track.org
- **Formats:** JSON, CSV, XML, HTML (CCSDS OMM format)

U.S. Space Surveillance Network data. Free registration required. Offers additional filtering by semi-major axis, period, apogee, and perigee.

## Telemetry / Instrument Data

### GOES-R Magnetometer Level 0 Data (Recommended)

- **Metadata:** https://www.ncei.noaa.gov/access/metadata/landing-page/bin/iso?id=gov.noaa.ncei.swx:mag-l0-goesr
- **Source:** NOAA NCEI

This is the closest thing to raw spacecraft telemetry publicly available from a GEO satellite. The Level 0 files contain CCSDS (Consultative Committee for Space Data Systems) packets with:

- Science telemetry (magnetometer measurements)
- Engineering telemetry (spacecraft housekeeping)
- Diagnostic telemetry
- Orbit and attitude data
- Eclipse-of-sun and yaw flip state data

Each CCSDS packet has a unique Application Process Identifier (APID). Recent archive tar files (14-day rolling window) are available on an anonymous FTP server. Older data is available by request through NCEI.

### NOAA GOES on AWS

- **Registry:** https://registry.opendata.aws/noaa-goes/
- **S3 buckets:** `s3://noaa-goes16/`, `s3://noaa-goes17/`, `s3://noaa-goes18/`

Full open access to GOES-16/17/18/19 products. No authentication required. Includes imagery, space weather products, and derived data. Useful if payload output data is needed.

### NASA Earthdata

- **URL:** https://www.earthdata.nasa.gov/
- **Search:** https://search.earthdata.nasa.gov/

Broad collection of Earth science data including GOES products. Free Earthdata Login required for downloads. Supports Aqua, Terra, GOES, NOAA, Suomi-NPP, Landsat, and many more.

### NASA GSFC GOES Data Servers

- **URL:** https://goes.gsfc.nasa.gov/text/goesds.html

Calibrated and navigated real-time GOES images in TIFF format at full resolution.

## Recommendation

For a realistic GEO satellite mock data setup:

1. **Orbital position** — Use a real GOES TLE from CelesTrak (`geo` or `goes` group). Feed into an SGP4 propagator (e.g., `satellite.js` for JS, `sgp4` for Python/C++) to compute position/velocity over time.
2. **Spacecraft telemetry** — Use GOES-R Level 0 MAG data from NOAA/NCEI. These CCSDS packets include engineering housekeeping (attitude, orbit, diagnostics) alongside science data, making them representative of a real satellite telemetry stream.
3. **Payload output** — Use GOES imagery or derived products from the AWS S3 buckets if visual/sensor payload data is needed.

## Propagation Libraries

For consuming TLE data:

| Library | Language | Notes |
|---|---|---|
| satellite.js | JavaScript/TypeScript | SGP4/SDP4, widely used for web apps |
| sgp4 | Python | pip-installable, wraps Vallado reference impl |
| libsgp4 | C++ | Native performance |
| Orekit | Java | Full astrodynamics library, TLE support |

## Data Access Verification

Tested 2026-02-02. Both recommended sources are live and accessible without authentication.

### CelesTrak GOES TLE (verified working)

```bash
curl -s 'https://celestrak.org/NORAD/elements/gp.php?GROUP=goes&FORMAT=json' -o data/goes_tle.json
```

Returns a JSON array of 20 GOES satellites (GOES 1 through GOES 19) with full OMM orbital elements. Each entry includes: `OBJECT_NAME`, `NORAD_CAT_ID`, `EPOCH`, `MEAN_MOTION`, `ECCENTRICITY`, `INCLINATION`, `RA_OF_ASC_NODE`, `ARG_OF_PERICENTER`, `MEAN_ANOMALY`, `BSTAR`, `MEAN_MOTION_DOT`, `MEAN_MOTION_DDOT`.

Active operational satellites for seed data:
- **GOES-16** (NORAD 41866) — GOES-East, inclination 0.15 deg
- **GOES-18** (NORAD 51850) — GOES-West, inclination 0.005 deg
- **GOES-19** (NORAD 60133) — newest, launched 2024, inclination 0.003 deg

To propagate into position/velocity time series: `pip install sgp4`, then use the JSON elements directly with `sgp4.api`.

### GOES-R MAG Level 0 (verified working)

Anonymous FTP with 14-day rolling window of daily tar files:

```
ftp://ftp.avl.class.noaa.gov/ddt/NCEI-NC/CS/GOESR-L0/MAG/
```

Each tar contains netCDF-4 files with CCSDS packet byte arrays. Includes science telemetry (magnetometer Bx/By/Bz), engineering housekeeping, orbit/attitude data, and eclipse state. Files are organized as daily bundles per satellite.

To parse: `pip install netCDF4`, extract tar, read netCDF variables.

## Notes

- GOES satellites are a joint NASA/NOAA program. NASA builds and launches; NOAA operates.
- Current constellation: GOES-16 (East), GOES-18 (West), GOES-17 (backup), GOES-19 (launched 2024).
- TDRS satellites are also in GEO but serve as communication relays — they do not produce publicly downloadable telemetry data.
- CelesTrak asks users to only download data when needed and not bulk-scrape.

## Implementation

Real GOES-16 data is integrated into the seed database via a fetch-and-cache pipeline. The generator (`data/gen_graphs.py`) reads cached JSON with stdlib only — no extra dependencies are needed to build the database. When cache files are absent, the generator falls back to fully synthetic data (preserving the original behavior).

### Architecture

```
  make fetch  (explicit, one-time)
       |
  +----+----+
  |         |
fetch_tle  fetch_mag
  |         |
  v         v
data/cache/goes_orbital.json    (stdlib json, ~100 records)
data/cache/goes_mag.json        (stdlib json, 100 x 256 samples)
       |
  gen_graphs.py  (reads cache with stdlib json if present)
       |
  data/lcars.db
```

### New Files

| File | Purpose |
|------|---------|
| `data/requirements.txt` | Python deps for fetchers: `sgp4>=2.22`, `netCDF4>=1.6` |
| `data/fetch_tle.py` | Downloads GOES TLE JSON from CelesTrak, finds GOES-16 (NORAD 41866), propagates with SGP4 over 24h at 100 points, computes altitude/longitude/range (Wallops Island ground station)/signal dBm (free-space path loss link budget). Writes `data/cache/goes_orbital.json`. |
| `data/fetch_mag.py` | Lists NOAA GOES-16 S3 bucket (`noaa-goes16/MAG-L1b-GEOF/2025/001/00/`), downloads one L1b netCDF file, extracts Bx/By magnetic field components, normalizes to -1.5..1.5, slices into 100 windows of 256 samples. Writes `data/cache/goes_mag.json`. |
| `data/cache/.gitkeep` | Tracks the cache directory in git (cache contents are gitignored). |

### Modified Files

| File | Changes |
|------|---------|
| `data/gen_graphs.py` | Added `json`/`os` imports and cache loading at top. Constant fields 45-50 (RADIO view) are overridden with real orbital data when cache is present: field 45 (GOES-16 longitude), 46 (signal dBm), 47 (noise floor derived from altitude), 48 (SNR), 49 (alignment status), 50 (range in km). Graph 0 ("Sensor Telemetry") uses real Bx/By magnetometer data when mag cache is present. All other fields/graphs unchanged. |
| `Makefile` | Added `fetch` target (runs both fetchers), `fetch-seed` target (fetch + reseed), and `$(wildcard data/cache/goes_*.json)` dependencies on the DB target so it auto-rebuilds when cache changes. |
| `.gitignore` | Added `data/cache/` with `!data/cache/.gitkeep` exception. |

### Fallback Behavior

| State | Result |
|-------|--------|
| No Python deps, no cache | Fully synthetic (original behavior) |
| Deps installed, no `make fetch` | Fully synthetic |
| Only TLE fetched | RADIO scalars use real data, graphs synthetic |
| Only MAG fetched | Graph 0 uses real data, RADIO scalars constant |
| Both fetched | RADIO scalars + graph 0 use real data |

### Usage

```bash
# Without real data (existing behavior preserved):
make data/lcars.db

# With real data:
pip install -r data/requirements.txt
make fetch-seed

# Or step by step:
python3 data/fetch_tle.py   # -> data/cache/goes_orbital.json
python3 data/fetch_mag.py   # -> data/cache/goes_mag.json
make reseed                 # rebuild DB using cached data

# Reset to synthetic:
rm -rf data/cache && make reseed
```

### Data Sources Used

1. **CelesTrak GOES TLE** — `GET https://celestrak.org/NORAD/elements/gp.php?GROUP=goes&FORMAT=json` — extracts GOES-16 (NORAD 41866), propagates with SGP4 to derive sub-satellite longitude, altitude, slant range, and received signal strength.
2. **GOES-16 MAG L1b from AWS S3** — `https://noaa-goes16.s3.amazonaws.com/MAG-L1b-GEOF/2025/001/00/` — calibrated magnetometer data in netCDF format. One file (~370KB, ~10 min coverage) provides Bx/By field components normalized into the graph Y-axis range.
