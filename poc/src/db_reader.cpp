#include "db_reader.h"
#include "field_defs.h"
#include "field_store.h"
#include "views/view_info.h"
#include "views/view_tactical.h"
#include "views/view_environ.h"
#include "views/view_power.h"
#include "views/view_radio.h"

#include <sqlite3.h>
#include "imgui/imgui.h"

#include <cmath>
#include <cstdio>
#include <ctime>
#include <span>

DbReader g_dbReader;

static const char* kScalarSQL =
    "SELECT field_id, numeric_value, string_value "
    "FROM scalar_data "
    "WHERE timestamp = ? "
    "ORDER BY field_id";

static const char* kGraphSQL =
    "SELECT graph_id, line_id, x, y "
    "FROM graph_data "
    "WHERE timestamp = ? "
    "ORDER BY graph_id, line_id, x";

bool DbReader::Open(const char* path) {
    int rc = sqlite3_open_v2(path, &db_, SQLITE_OPEN_READONLY, nullptr);
    if (rc != SQLITE_OK) {
        std::fprintf(stderr, "DbReader: cannot open '%s': %s\n",
                     path, sqlite3_errmsg(db_));
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }
    PrepareStatements();
    LoadTimestamps();
    return true;
}

void DbReader::Close() {
    FinalizeStatements();
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
    timestamps_.clear();
    currentIdx_ = 0;
}

bool DbReader::IsOpen() const {
    return db_ != nullptr;
}

void DbReader::PrepareStatements() {
    int rc1 = sqlite3_prepare_v2(db_, kScalarSQL, -1, &scalarStmt_, nullptr);
    if (rc1 != SQLITE_OK)
        std::fprintf(stderr, "DbReader: scalar prepare failed: %s\n", sqlite3_errmsg(db_));
    int rc2 = sqlite3_prepare_v2(db_, kGraphSQL, -1, &graphStmt_, nullptr);
    if (rc2 != SQLITE_OK)
        std::fprintf(stderr, "DbReader: graph prepare failed: %s\n", sqlite3_errmsg(db_));
}

void DbReader::FinalizeStatements() {
    if (scalarStmt_) { sqlite3_finalize(scalarStmt_); scalarStmt_ = nullptr; }
    if (graphStmt_)  { sqlite3_finalize(graphStmt_);  graphStmt_ = nullptr;  }
}

void DbReader::LoadTimestamps() {
    const char* sql = "SELECT DISTINCT timestamp FROM scalar_data ORDER BY timestamp";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    while (sqlite3_step(stmt) == SQLITE_ROW)
        timestamps_.push_back(sqlite3_column_double(stmt, 0));
    sqlite3_finalize(stmt);
    std::printf("  %d timestamps loaded (%.1fs of data)\n",
                (int)timestamps_.size(),
                timestamps_.empty() ? 0.0 : timestamps_.back() - timestamps_.front());
}

void DbReader::PollIfNeeded() {
    double now = ImGui::GetTime();
    if (now - lastQueryTime_ < pollIntervalSec_)
        return;
    lastQueryTime_ = now;
    QueryAndUpdate();
}

void DbReader::QueryAndUpdate() {
    if (timestamps_.empty()) return;

    double ts = timestamps_[currentIdx_];
    currentIdx_ = (currentIdx_ + 1) % (int)timestamps_.size();

    LoadScalars(ts);
    LoadGraphs(ts);
    FormatAllFields();
}

void DbReader::LoadScalars(double ts) {
    if (!scalarStmt_) return;
    sqlite3_reset(scalarStmt_);
    sqlite3_bind_double(scalarStmt_, 1, ts);

    while (sqlite3_step(scalarStmt_) == SQLITE_ROW) {
        int fid = sqlite3_column_int(scalarStmt_, 0);
        if (fid < 0 || fid >= (int)FieldId::COUNT) continue;

        FieldId id = (FieldId)fid;

        if (sqlite3_column_type(scalarStmt_, 1) != SQLITE_NULL) {
            double numVal = sqlite3_column_double(scalarStmt_, 1);
            g_dbFields.Set(id, numVal);
        }

        if (sqlite3_column_type(scalarStmt_, 2) != SQLITE_NULL) {
            const char* strVal = (const char*)sqlite3_column_text(scalarStmt_, 2);
            if (strVal)
                g_dbFields.SetString(id, strVal);
        }
    }
}

void DbReader::LoadGraphs(double ts) {
    if (!graphStmt_) return;
    sqlite3_reset(graphStmt_);
    sqlite3_bind_double(graphStmt_, 1, ts);

    constexpr int kTotalGraphs = static_cast<int>(GraphId::COUNT);
    int idx[kTotalGraphs][2] = {};

    while (sqlite3_step(graphStmt_) == SQLITE_ROW) {
        int graphId = sqlite3_column_int(graphStmt_, 0);
        int lineId  = sqlite3_column_int(graphStmt_, 1);
        float x     = (float)sqlite3_column_double(graphStmt_, 2);
        float y     = (float)sqlite3_column_double(graphStmt_, 3);

        if (graphId < 0 || graphId >= kTotalGraphs) continue;
        if (lineId < 0 || lineId > 1) continue;

        int i = idx[graphId][lineId];
        if (i >= kGraphSamples) continue;

        g_dbGraphBufs[graphId].xs[i] = x;
        g_dbGraphBufs[graphId].ys[lineId][i] = y;
        idx[graphId][lineId]++;
    }
}

template<typename FieldDefT>
static void FormatFields(std::span<const FieldDefT> fields) {
    for (const auto& fd : fields) {
        FieldId fid = fd.GetFieldId();

        if (fd.display == Display::TimerUp || fd.display == Display::TimerDown) {
            double seconds = g_dbFields.Get(fid);
            if (seconds < 0) seconds = 0;
            int d = (int)(seconds / 86400.0);
            int h = (int)(std::fmod(seconds, 86400.0) / 3600.0);
            int m = (int)(std::fmod(seconds, 3600.0) / 60.0);
            int s = (int)(std::fmod(seconds, 60.0));
            std::snprintf(g_dbFields.strings[(int)fid], sizeof(g_dbFields.strings[0]),
                          "%03dD %02dH %02dM %02dS", d, h, m, s);
        }
        else if (fd.display == Display::Clock) {
            double epoch = g_dbFields.Get(fid);
            std::time_t tt = (std::time_t)epoch;
            struct std::tm* tm = std::gmtime(&tt);
            if (tm) {
                std::snprintf(g_dbFields.strings[(int)fid], sizeof(g_dbFields.strings[0]),
                              "%04d.%02d.%02d  %02d:%02d:%02d",
                              tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                              tm->tm_hour, tm->tm_min, tm->tm_sec);
            }
        }
    }
}

void DbReader::FormatAllFields() {
    FormatFields<InfoFieldDef>(info_detail::kFields);
    FormatFields<TacticalFieldDef>(tactical_detail::kFields);
    FormatFields<EnvironFieldDef>(environ_detail::kFields);
    FormatFields<PowerFieldDef>(power_detail::kFields);
    FormatFields<RadioFieldDef>(radio_detail::kFields);
}
