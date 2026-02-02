#!/usr/bin/env python3
"""Generate scalar_data and graph_data INSERT statements for the LCARS seed DB.

Produces 100 timestamps (0.0 to 9.9 at 0.1s intervals) of:
  - 51 scalar rows per timestamp  (timers tick, clocks advance, others constant)
  - 2048 graph rows per timestamp (4 graphs x 2 lines x 256 samples, waves evolve)

Usage:
    (cat data/seed.sql && python3 data/gen_graphs.py) | sqlite3 data/lcars.db
"""

import math

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------
NUM_STEPS = 100
DT = 0.1  # seconds between timestamps

GRAPH_SAMPLES = 256
GRAPH_X_MAX = 10.0

# Base values matching demo mode at t=0
MISSION_ELAPSED_BASE = 127 * 86400 + 4 * 3600 + 32 * 60  # 10989120 s
ECLIPSE_BASE = 12.3 * 86400   # 1062720 s
MAINT_BASE = 3.8 * 86400      # 328320 s
CLOCK_EPOCH = 1700000000.0
STARDATE = "42048.7"

# Constant scalar fields: (field_id, string_value)
CONSTANT_FIELDS = [
    (7,  "ONLINE  -  OUTPUT 98.7%"),
    (8,  "ACTIVE  -  STRENGTH 100%"),
    (9,  "NOMINAL -  97.3%"),
    (10, "NOMINAL -  ALL DECKS"),
    (11, "22.4 C / 72.3 F"),
    (12, "47.2%"),
    (13, "101.3 kPa"),
    (14, "98"),
    (15, "412 PPM"),
    (16, "20.9%"),
    (17, "22.1 C  NOMINAL"),
    (18, "22.3 C  NOMINAL"),
    (19, "22.0 C  NOMINAL"),
    (20, "21.8 C  NOMINAL"),
    (21, "24.7 C  ELEVATED"),
    (22, "19.2 C  NOMINAL"),
    (23, "1,247.3 MW"),
    (24, "312.8 MW"),
    (25, "47.2 MW"),
    (26, "89.1 MW"),
    (27, "1,696.4 MW"),
    (28, "847.2 MW"),
    (29, "198.4 MW"),
    (30, "112.7 MW"),
    (31, "78.3 MW"),
    (32, "64.1 MW"),
    (33, "0.0 MW  STANDBY"),
    (34, "1,300.7 MW"),
    (35, "98.2%"),
    (36, "97.8%"),
    (37, "100%"),
    (38, "395.7 MW"),
    (39, "47.3 GHz  ACTIVE"),
    (40, "121.5 MHz  MONITORING"),
    (41, "89.7 GHz  STANDBY"),
    (42, "22.1 GHz  ACTIVE"),
    (43, "63.4 GHz  ENCRYPTED"),
    (44, "41.8 GHz  STANDBY"),
    (45, "47.3 GHz"),
    (46, "-42 dBm"),
    (47, "-110 dBm"),
    (48, "68 dB"),
    (49, "ALIGNED"),
    (50, "4.7 LY"),
]

# Graph wave params: (base, xf1, tf1, a1, p1, xf2, tf2, a2, p2)
GRAPHS = [
    (0, [
        (0, (0.0,    1.0, 1.0, 0.5, 0.0,       2.3,  1.7, 0.5, 0.0)),
        (1, (0.0,    0.7, 0.8, 0.8, math.pi/2,  0.0,  0.0, 0.0, 0.0)),
    ]),
    (1, [
        (0, (22.0,   0.5, 0.3, 0.5, 0.0,       0.0,  0.0, 0.0, 0.0)),
        (1, (47.0,   0.3, 0.2, 2.0, math.pi/2,  0.0,  0.0, 0.0, 0.0)),
    ]),
    (2, [
        (0, (1300.0, 0.8, 0.4, 50.0, 0.0,      0.0,  0.0, 0.0, 0.0)),
        (1, (1696.0, 0.6, 0.3, 30.0, math.pi/2, 0.0,  0.0, 0.0, 0.0)),
    ]),
    (3, [
        (0, (-42.0,  2.0, 1.5, 8.0, 0.0,       7.3,  3.1, 3.0, 0.0)),
        (1, (-110.0, 3.0, 0.7, 2.0, 0.0,      11.0,  2.3, 1.5, math.pi/2)),
    ]),
]


def wave(x, t, params):
    base, xf1, tf1, a1, p1, xf2, tf2, a2, p2 = params
    return (base
            + math.sin(x * xf1 + t * tf1 + p1) * a1
            + math.sin(x * xf2 + t * tf2 + p2) * a2)


def sq(s):
    """Escape single quotes for SQL."""
    return s.replace("'", "''")


# ---------------------------------------------------------------------------
# Generate
# ---------------------------------------------------------------------------
print("BEGIN;")

for step in range(NUM_STEPS):
    ts = round(step * DT, 1)
    t = ts  # simulation time

    # --- Timers (numeric_value) ---
    print(f"INSERT INTO scalar_data VALUES({ts},0,{MISSION_ELAPSED_BASE + t},NULL);")
    print(f"INSERT INTO scalar_data VALUES({ts},1,{t},NULL);")
    print(f"INSERT INTO scalar_data VALUES({ts},2,{max(ECLIPSE_BASE - t, 0)},NULL);")
    print(f"INSERT INTO scalar_data VALUES({ts},3,{max(MAINT_BASE - t, 0)},NULL);")

    # --- Clocks (numeric_value) ---
    print(f"INSERT INTO scalar_data VALUES({ts},4,{CLOCK_EPOCH + t},NULL);")
    print(f"INSERT INTO scalar_data VALUES({ts},5,{CLOCK_EPOCH + t},NULL);")

    # --- Stardate (string_value) ---
    print(f"INSERT INTO scalar_data VALUES({ts},6,NULL,'{STARDATE}');")

    # --- Constant scalar fields ---
    for fid, val in CONSTANT_FIELDS:
        print(f"INSERT INTO scalar_data VALUES({ts},{fid},NULL,'{sq(val)}');")

    # --- Graph data ---
    for graph_id, lines in GRAPHS:
        for line_id, params in lines:
            for i in range(GRAPH_SAMPLES):
                x = i / (GRAPH_SAMPLES - 1) * GRAPH_X_MAX
                y = wave(x, t, params)
                print(f"INSERT INTO graph_data VALUES({ts},{graph_id},"
                      f"{line_id},{x:.6f},{y:.6f});")

print("COMMIT;")
