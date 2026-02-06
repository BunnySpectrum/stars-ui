#!/usr/bin/env python3
"""Fetch GOES-16 TLE from CelesTrak and propagate orbit over 24 hours.

Writes:
  data/cache/goes_tle.json      — raw TLE JSON from CelesTrak
  data/cache/goes_orbital.json  — 100 propagated samples with
      {ts, alt_km, lon_deg, range_km, signal_dbm}

Requires: sgp4 (pip install sgp4)
"""

import json
import math
import os
import urllib.request

CACHE_DIR = os.path.join(os.path.dirname(__file__), "cache")
TLE_URL = "https://celestrak.org/NORAD/elements/gp.php?GROUP=goes&FORMAT=json"
TLE_CACHE = os.path.join(CACHE_DIR, "goes_tle.json")
ORBITAL_CACHE = os.path.join(CACHE_DIR, "goes_orbital.json")

GOES16_NORAD = 41866
NUM_SAMPLES = 100
PROPAGATION_HOURS = 24

# Wallops Island ground station (approx)
GS_LAT = 37.9402   # degrees N
GS_LON = -75.4664  # degrees W
GS_ALT = 0.0       # km above sea level

# Link budget constants for signal strength estimate
EIRP_DBW = 52.0          # typical GOES downlink EIRP
FREQ_HZ = 1694.1e6       # GOES rebroadcast frequency
BOLTZMANN_DB = -228.6     # 10*log10(k) in dBW/K/Hz
ANTENNA_GAIN_DB = 30.0    # ground antenna gain estimate
SYSTEM_TEMP_K_DB = 23.0   # 10*log10(200K) system noise temp


def fetch_tle():
    """Download TLE JSON from CelesTrak, cache locally."""
    os.makedirs(CACHE_DIR, exist_ok=True)
    if os.path.exists(TLE_CACHE):
        print(f"TLE cache exists: {TLE_CACHE}")
        with open(TLE_CACHE) as f:
            return json.load(f)

    print(f"Downloading TLE from {TLE_URL} ...")
    req = urllib.request.Request(TLE_URL, headers={"User-Agent": "stars-fetch/1.0"})
    with urllib.request.urlopen(req, timeout=30) as resp:
        data = json.loads(resp.read().decode())

    with open(TLE_CACHE, "w") as f:
        json.dump(data, f, indent=2)
    print(f"Cached {len(data)} satellites to {TLE_CACHE}")
    return data


def find_goes16(tle_list):
    """Find GOES-16 entry by NORAD catalog number."""
    for sat in tle_list:
        if sat.get("NORAD_CAT_ID") == GOES16_NORAD:
            return sat
    raise ValueError(f"GOES-16 (NORAD {GOES16_NORAD}) not found in TLE data")


def propagate(sat_record):
    """Propagate GOES-16 orbit and compute derived quantities."""
    from sgp4.api import Satrec, WGS72
    from sgp4.api import jday

    satrec = Satrec()
    satrec.sgp4init(
        WGS72,
        'i',  # improved mode
        sat_record["NORAD_CAT_ID"],
        (sat_record["EPOCH"] if isinstance(sat_record["EPOCH"], float)
         else _epoch_to_jd_offset(sat_record)),
        sat_record["BSTAR"],
        0.0,  # ndot (not used in SGP4 init this way)
        0.0,  # nddot
        sat_record["ECCENTRICITY"],
        math.radians(sat_record["ARG_OF_PERICENTER"]),
        math.radians(sat_record["INCLINATION"]),
        math.radians(sat_record["MEAN_ANOMALY"]),
        sat_record["MEAN_MOTION"] * (2.0 * math.pi / 1440.0),  # rev/day -> rad/min
        math.radians(sat_record["RA_OF_ASC_NODE"]),
    )

    # Start from TLE epoch
    epoch_str = sat_record["EPOCH"]
    year = int(epoch_str[:4])
    month = int(epoch_str[5:7])
    day = int(epoch_str[8:10])
    hour = int(epoch_str[11:13])
    minute = int(epoch_str[14:16])
    second = float(epoch_str[17:])

    jd_base, fr_base = jday(year, month, day, hour, minute, second)

    results = []
    for i in range(NUM_SAMPLES):
        # Spread over PROPAGATION_HOURS
        dt_min = i * (PROPAGATION_HOURS * 60.0) / (NUM_SAMPLES - 1)

        # Add dt to fractional day
        jd = jd_base
        fr = fr_base + dt_min / 1440.0

        e, r, v = satrec.sgp4(jd, fr)
        if e != 0:
            print(f"  SGP4 error {e} at sample {i}, skipping")
            continue

        # r is ECI position in km
        x, y, z = r

        # Compute altitude (distance from Earth center minus Earth radius)
        r_mag = math.sqrt(x * x + y * y + z * z)
        alt_km = r_mag - 6378.137

        # Approximate sub-satellite longitude
        # Greenwich sidereal time
        gmst = _gmst(jd, fr)
        lon_deg = math.degrees(math.atan2(y, x)) - gmst
        # Normalize to -180..180
        lon_deg = ((lon_deg + 180) % 360) - 180

        # Range from ground station
        gs_x, gs_y, gs_z = _geodetic_to_eci(GS_LAT, GS_LON, GS_ALT, gmst)
        dx = x - gs_x
        dy = y - gs_y
        dz = z - gs_z
        range_km = math.sqrt(dx * dx + dy * dy + dz * dz)

        # Signal strength via free-space path loss
        signal_dbm = _link_budget(range_km)

        ts = round(i * 9.9 / (NUM_SAMPLES - 1), 1)

        results.append({
            "ts": ts,
            "alt_km": round(alt_km, 2),
            "lon_deg": round(lon_deg, 4),
            "range_km": round(range_km, 2),
            "signal_dbm": round(signal_dbm, 1),
        })

    return results


def _epoch_to_jd_offset(sat):
    """Compute epoch as Julian date offset for sgp4init (days since 1949-12-31)."""
    epoch_str = sat["EPOCH"]
    year = int(epoch_str[:4])
    month = int(epoch_str[5:7])
    day = int(epoch_str[8:10])
    hour = int(epoch_str[11:13])
    minute = int(epoch_str[14:16])
    second = float(epoch_str[17:])

    from sgp4.api import jday
    jd, fr = jday(year, month, day, hour, minute, second)
    # sgp4init epoch is days since 1949 Dec 31 0h UT
    return (jd + fr) - 2433281.5


def _gmst(jd, fr):
    """Approximate Greenwich Mean Sidereal Time in degrees."""
    t_ut1 = ((jd - 2451545.0) + fr) / 36525.0
    gmst_sec = (67310.54841
                + (876600.0 * 3600 + 8640184.812866) * t_ut1
                + 0.093104 * t_ut1 * t_ut1
                - 6.2e-6 * t_ut1 * t_ut1 * t_ut1)
    gmst_deg = (gmst_sec / 240.0) % 360.0
    return gmst_deg


def _geodetic_to_eci(lat_deg, lon_deg, alt_km, gmst_deg):
    """Convert geodetic coordinates to ECI (approximate, spherical Earth)."""
    lat = math.radians(lat_deg)
    lon = math.radians(lon_deg + gmst_deg)
    r = 6378.137 + alt_km
    x = r * math.cos(lat) * math.cos(lon)
    y = r * math.cos(lat) * math.sin(lon)
    z = r * math.sin(lat)
    return x, y, z


def _link_budget(range_km):
    """Free-space path loss link budget -> received signal in dBm."""
    range_m = range_km * 1000.0
    wavelength = 3e8 / FREQ_HZ
    fspl_db = 20 * math.log10(4 * math.pi * range_m / wavelength)
    received_dbw = EIRP_DBW + ANTENNA_GAIN_DB - fspl_db
    return received_dbw + 30  # dBW -> dBm


def main():
    tle_list = fetch_tle()
    goes16 = find_goes16(tle_list)
    print(f"Found GOES-16: {goes16['OBJECT_NAME']}")

    print(f"Propagating {NUM_SAMPLES} samples over {PROPAGATION_HOURS}h ...")
    orbital = propagate(goes16)
    print(f"Generated {len(orbital)} orbital samples")

    if orbital:
        print(f"  Alt range: {min(r['alt_km'] for r in orbital):.1f} - "
              f"{max(r['alt_km'] for r in orbital):.1f} km")
        print(f"  Lon range: {min(r['lon_deg'] for r in orbital):.4f} - "
              f"{max(r['lon_deg'] for r in orbital):.4f} deg")
        print(f"  Signal range: {min(r['signal_dbm'] for r in orbital):.1f} - "
              f"{max(r['signal_dbm'] for r in orbital):.1f} dBm")

    with open(ORBITAL_CACHE, "w") as f:
        json.dump(orbital, f, indent=2)
    print(f"Wrote {ORBITAL_CACHE}")


if __name__ == "__main__":
    main()
