#pragma once

#include <vector>

// Forward declarations — sqlite3.h is only included in db_reader.cpp
struct sqlite3;
struct sqlite3_stmt;

class DbReader {
public:
    bool Open(const char* path);
    void Close();
    bool IsOpen() const;
    void PollIfNeeded();

private:
    sqlite3* db_ = nullptr;
    sqlite3_stmt* scalarStmt_ = nullptr;
    sqlite3_stmt* graphStmt_ = nullptr;
    double lastQueryTime_ = -999.0;
    double pollIntervalSec_ = 0.1;

    std::vector<double> timestamps_;
    int currentIdx_ = 0;

    void PrepareStatements();
    void FinalizeStatements();
    void LoadTimestamps();
    void QueryAndUpdate();
    void LoadScalars(double ts);
    void LoadGraphs(double ts);
    void FormatAllFields();
};

extern DbReader g_dbReader;
