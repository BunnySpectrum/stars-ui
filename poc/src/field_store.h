#pragma once

#include "field_defs.h"
#include "graph_buffer.h"

#include <cstdio>
#include <cstring>
#include <span>

struct FieldStore {
    double scalars[(int)FieldId::COUNT] = {};
    char   strings[(int)FieldId::COUNT][128] = {};

    void Set(FieldId id, double v) { scalars[(int)id] = v; }
    double Get(FieldId id) const { return scalars[(int)id]; }

    void SetString(FieldId id, const char* s) {
        std::snprintf(strings[(int)id], sizeof(strings[0]), "%s", s);
    }
    const char* GetString(FieldId id) const { return strings[(int)id]; }
};

// Global field store and graph buffers populated by the database
extern FieldStore g_dbFields;
extern GraphBuffer g_dbGraphBufs[static_cast<int>(GraphId::COUNT)];

// Per-view update functions
void UpdateInfoFields(FieldStore& fields, std::span<GraphBuffer> graphBufs);
void UpdateTacticalFields(FieldStore& fields, std::span<GraphBuffer> graphBufs);
void UpdateShipFields(FieldStore& fields, std::span<GraphBuffer> graphBufs);
void UpdateEnvironFields(FieldStore& fields, std::span<GraphBuffer> graphBufs);
void UpdatePowerFields(FieldStore& fields, std::span<GraphBuffer> graphBufs);
void UpdateRadioFields(FieldStore& fields, std::span<GraphBuffer> graphBufs);
void UpdateMapFields(FieldStore& fields, std::span<GraphBuffer> graphBufs);
void UpdateRfFields(FieldStore& fields, std::span<GraphBuffer> graphBufs);
