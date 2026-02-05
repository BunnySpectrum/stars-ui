#pragma once

#include "field_defs.h"

#include <cstdio>
#include <cstring>

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

struct GraphBuffer {
    float xs[kGraphSamples];
    float ys[2][kGraphSamples];
};

// Global instances
extern FieldStore g_fields;
extern GraphBuffer g_graphBufs[static_cast<int>(GraphId::COUNT)];

// When true, UpdateFieldStore() is a no-op — data comes from the database.
extern bool g_useDatabase;

// Call once per frame before rendering to update all field values and graph buffers.
void UpdateFieldStore();
