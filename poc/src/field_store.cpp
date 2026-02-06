#include "field_store.h"
#include "views/view_tactical.h"
#include "views/view_environ.h"
#include "views/view_power.h"
#include "views/view_radio.h"

// Global field store and graph buffers populated by the database
FieldStore g_dbFields;
GraphBuffer g_dbGraphBufs[static_cast<int>(GraphId::COUNT)];

// ============================================================================
// Per-view update functions — copy fields/graphs from global database buffers
// ============================================================================

void UpdateInfoFields(FieldStore& fields, std::span<GraphBuffer> graphBufs) {
    fields = g_dbFields;
}

void UpdateTacticalFields(FieldStore& fields, std::span<GraphBuffer> graphBufs) {
    fields = g_dbFields;
    for (size_t i = 0; i < kTacticalContent.graphs.size() && i < graphBufs.size(); i++) {
        GraphId gid = kTacticalContent.graphs[i].id;
        graphBufs[i] = g_dbGraphBufs[static_cast<int>(gid)];
    }
}

void UpdateShipFields(FieldStore& fields, std::span<GraphBuffer> graphBufs) {
    fields = g_dbFields;
}

void UpdateEnvironFields(FieldStore& fields, std::span<GraphBuffer> graphBufs) {
    fields = g_dbFields;
    for (size_t i = 0; i < kEnvironContent.graphs.size() && i < graphBufs.size(); i++) {
        GraphId gid = kEnvironContent.graphs[i].id;
        graphBufs[i] = g_dbGraphBufs[static_cast<int>(gid)];
    }
}

void UpdatePowerFields(FieldStore& fields, std::span<GraphBuffer> graphBufs) {
    fields = g_dbFields;
    for (size_t i = 0; i < kPowerContent.graphs.size() && i < graphBufs.size(); i++) {
        GraphId gid = kPowerContent.graphs[i].id;
        graphBufs[i] = g_dbGraphBufs[static_cast<int>(gid)];
    }
}

void UpdateRadioFields(FieldStore& fields, std::span<GraphBuffer> graphBufs) {
    fields = g_dbFields;
    for (size_t i = 0; i < kRadioContent.graphs.size() && i < graphBufs.size(); i++) {
        GraphId gid = kRadioContent.graphs[i].id;
        graphBufs[i] = g_dbGraphBufs[static_cast<int>(gid)];
    }
}

void UpdateMapFields(FieldStore& fields, std::span<GraphBuffer> graphBufs) {
    fields = g_dbFields;
}

void UpdateRfFields(FieldStore& fields, std::span<GraphBuffer> graphBufs) {
    fields = g_dbFields;
}
