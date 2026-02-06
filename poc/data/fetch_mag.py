#!/usr/bin/env python3
"""Fetch GOES-16 MAG L1b data from NOAA S3 and extract magnetometer samples.

Writes:
  data/cache/goes_mag.json — 100 windows of 256 samples each:
      {ts, xs, line0, line1}

Requires: netCDF4 (pip install netCDF4)
"""

import json
import math
import os
import re
import urllib.request
import xml.etree.ElementTree as ET

CACHE_DIR = os.path.join(os.path.dirname(__file__), "cache")
MAG_CACHE = os.path.join(CACHE_DIR, "goes_mag.json")

S3_BUCKET = "noaa-goes16"
S3_PREFIX = "MAG-L1b-GEOF/2025/001/00/"
S3_BASE = f"https://{S3_BUCKET}.s3.amazonaws.com"
S3_LIST_URL = f"{S3_BASE}?prefix={S3_PREFIX}&max-keys=10"

NUM_WINDOWS = 100
SAMPLES_PER_WINDOW = 256
GRAPH_X_MAX = 10.0
Y_MIN = -1.5
Y_MAX = 1.5


def find_nc_file():
    """List S3 bucket to find a .nc file in the MAG prefix."""
    print(f"Listing S3: {S3_LIST_URL} ...")
    req = urllib.request.Request(S3_LIST_URL, headers={"User-Agent": "stars-fetch/1.0"})
    with urllib.request.urlopen(req, timeout=30) as resp:
        xml_data = resp.read()

    root = ET.fromstring(xml_data)
    ns = {"s3": "http://s3.amazonaws.com/doc/2006-03-01/"}

    keys = []
    for contents in root.findall("s3:Contents", ns):
        key = contents.find("s3:Key", ns)
        if key is not None and key.text.endswith(".nc"):
            keys.append(key.text)

    if not keys:
        raise RuntimeError(f"No .nc files found at s3://{S3_BUCKET}/{S3_PREFIX}")

    # Pick first file
    return keys[0]


def download_nc(s3_key):
    """Download a netCDF file from S3 to the cache directory."""
    os.makedirs(CACHE_DIR, exist_ok=True)
    filename = os.path.basename(s3_key)
    local_path = os.path.join(CACHE_DIR, filename)

    if os.path.exists(local_path):
        print(f"NC file cached: {local_path}")
        return local_path

    url = f"{S3_BASE}/{s3_key}"
    print(f"Downloading {url} ...")
    req = urllib.request.Request(url, headers={"User-Agent": "stars-fetch/1.0"})
    with urllib.request.urlopen(req, timeout=60) as resp:
        data = resp.read()

    with open(local_path, "wb") as f:
        f.write(data)
    print(f"Cached {len(data)} bytes to {local_path}")
    return local_path


def extract_mag(nc_path):
    """Read Bx/By from netCDF and produce windowed, normalized samples."""
    from netCDF4 import Dataset

    ds = Dataset(nc_path, "r")

    # L1b MAG files typically have variables like:
    #   b_gse (3-component GSE magnetic field) or
    #   b_total, bx_gse, by_gse, bz_gse
    # Try common variable names
    bx = None
    by = None

    # Try GSE components
    for bx_name in ["bx_gse", "b_gse_x"]:
        if bx_name in ds.variables:
            bx = ds.variables[bx_name][:].flatten()
            break

    for by_name in ["by_gse", "b_gse_y"]:
        if by_name in ds.variables:
            by = ds.variables[by_name][:].flatten()
            break

    # Try combined b_gse array (shape: [time, 3])
    if bx is None and "b_gse" in ds.variables:
        b_gse = ds.variables["b_gse"][:]
        if b_gse.ndim >= 2 and b_gse.shape[-1] >= 2:
            bx = b_gse[:, 0].flatten()
            by = b_gse[:, 1].flatten()

    if bx is None or by is None:
        # List available variables for debugging
        var_names = list(ds.variables.keys())
        ds.close()
        raise RuntimeError(
            f"Could not find Bx/By variables in {nc_path}. "
            f"Available: {var_names}"
        )

    ds.close()

    print(f"Extracted Bx ({len(bx)} samples), By ({len(by)} samples)")

    # Normalize both to Y_MIN..Y_MAX range
    bx = _normalize(bx, Y_MIN, Y_MAX)
    by = _normalize(by, Y_MIN, Y_MAX)

    # Slice into windows
    total_needed = NUM_WINDOWS * SAMPLES_PER_WINDOW
    if len(bx) < total_needed:
        # Tile to fill
        repeats = math.ceil(total_needed / len(bx))
        bx = (list(bx) * repeats)[:total_needed]
        by = (list(by) * repeats)[:total_needed]
    else:
        bx = list(bx[:total_needed])
        by = list(by[:total_needed])

    results = []
    for w in range(NUM_WINDOWS):
        ts = round(w * 9.9 / (NUM_WINDOWS - 1), 1)
        start = w * SAMPLES_PER_WINDOW
        end = start + SAMPLES_PER_WINDOW

        xs = [round(i / (SAMPLES_PER_WINDOW - 1) * GRAPH_X_MAX, 6)
              for i in range(SAMPLES_PER_WINDOW)]

        results.append({
            "ts": ts,
            "xs": xs,
            "line0": [round(float(v), 6) for v in bx[start:end]],
            "line1": [round(float(v), 6) for v in by[start:end]],
        })

    return results


def _normalize(arr, target_min, target_max):
    """Normalize array to target range, handling masked/nan values."""
    import numpy as np

    arr = np.array(arr, dtype=float)

    # Handle masked arrays
    if hasattr(arr, 'mask'):
        arr = np.where(arr.mask, 0.0, arr.data)

    # Replace NaN/Inf
    arr = np.where(np.isfinite(arr), arr, 0.0)

    src_min = float(np.min(arr))
    src_max = float(np.max(arr))

    if src_max - src_min < 1e-10:
        return np.full_like(arr, (target_min + target_max) / 2.0)

    return target_min + (arr - src_min) / (src_max - src_min) * (target_max - target_min)


def main():
    if os.path.exists(MAG_CACHE):
        print(f"MAG cache exists: {MAG_CACHE}")
        return

    s3_key = find_nc_file()
    print(f"Selected: {s3_key}")

    nc_path = download_nc(s3_key)
    mag_data = extract_mag(nc_path)

    with open(MAG_CACHE, "w") as f:
        json.dump(mag_data, f)
    print(f"Wrote {len(mag_data)} windows to {MAG_CACHE}")


if __name__ == "__main__":
    main()
