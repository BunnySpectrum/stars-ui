-- LCARS seed schema

CREATE TABLE scalar_data (
    timestamp     REAL    NOT NULL,
    field_id      INTEGER NOT NULL,
    numeric_value REAL,
    string_value  TEXT
);

CREATE TABLE graph_data (
    timestamp REAL    NOT NULL,
    graph_id  INTEGER NOT NULL,
    line_id   INTEGER NOT NULL,
    x         REAL    NOT NULL,
    y         REAL    NOT NULL
);

CREATE INDEX idx_scalar_ts ON scalar_data(timestamp);
CREATE INDEX idx_graph_ts  ON graph_data(timestamp);
