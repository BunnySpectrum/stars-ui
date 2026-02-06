#include "field_store.h"
#include "views/view_tactical.h"
#include "views/view_environ.h"
#include "views/view_power.h"
#include "views/view_radio.h"

#include <cmath>
#include <ctime>
#include <cstdio>

FieldStore  g_fields;
GraphBuffer g_graphBufs[static_cast<int>(GraphId::COUNT)];
bool        g_useDatabase = false;

static void FormatTimer(FieldId id, double seconds) {
    if (seconds < 0) seconds = 0;
    int d = (int)(seconds / 86400.0);
    int h = (int)(std::fmod(seconds, 86400.0) / 3600.0);
    int m = (int)(std::fmod(seconds, 3600.0) / 60.0);
    int s = (int)(std::fmod(seconds, 60.0));
    std::snprintf(g_fields.strings[(int)id], sizeof(g_fields.strings[0]),
                  "%03dD %02dH %02dM %02dS", d, h, m, s);
}

static void FormatClock(FieldId id, struct std::tm* tm) {
    std::snprintf(g_fields.strings[(int)id], sizeof(g_fields.strings[0]),
                  "%04d.%02d.%02d  %02d:%02d:%02d",
                  tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                  tm->tm_hour, tm->tm_min, tm->tm_sec);
}

void UpdateFieldStore() {
    if (g_useDatabase) return;

    double t = ImGui::GetTime();
    std::time_t now = std::time(nullptr);

    // === Timers ===
    double missionElapsed = t + 127.0 * 86400.0 + 4.0 * 3600.0 + 32.0 * 60.0;
    g_fields.Set(FieldId::MissionElapsed, missionElapsed);
    FormatTimer(FieldId::MissionElapsed, missionElapsed);

    g_fields.Set(FieldId::Uptime, t);
    FormatTimer(FieldId::Uptime, t);

    double eclipse = 12.3 * 86400.0 - t;
    if (eclipse < 0) eclipse = 0;
    g_fields.Set(FieldId::TimeToEclipse, eclipse);
    FormatTimer(FieldId::TimeToEclipse, eclipse);

    double maint = 3.8 * 86400.0 - t;
    if (maint < 0) maint = 0;
    g_fields.Set(FieldId::NextMaintenance, maint);
    FormatTimer(FieldId::NextMaintenance, maint);

    // === Clocks ===
    g_fields.Set(FieldId::PacificTime, (double)now);
    struct std::tm* local = std::localtime(&now);
    FormatClock(FieldId::PacificTime, local);

    g_fields.Set(FieldId::Utc, (double)now);
    struct std::tm* utc = std::gmtime(&now);
    FormatClock(FieldId::Utc, utc);

    double stardate = 41000.0 + (double)(now - 1609459200) / 86400.0 * 0.1;
    g_fields.Set(FieldId::Stardate, stardate);
    std::snprintf(g_fields.strings[(int)FieldId::Stardate], 128, "%.1f", stardate);

    // === Tactical ===
    g_fields.Set(FieldId::TacWarpCore, 98.7);
    g_fields.SetString(FieldId::TacWarpCore,      "ONLINE  -  OUTPUT 98.7%");
    g_fields.Set(FieldId::TacShields, 100.0);
    g_fields.SetString(FieldId::TacShields,       "ACTIVE  -  STRENGTH 100%");
    g_fields.Set(FieldId::TacHullIntegrity, 97.3);
    g_fields.SetString(FieldId::TacHullIntegrity, "NOMINAL -  97.3%");
    g_fields.Set(FieldId::TacLifeSupport, 100.0);
    g_fields.SetString(FieldId::TacLifeSupport,   "NOMINAL -  ALL DECKS");

    // === Environ — Atmospheric ===
    g_fields.SetString(FieldId::EnvTemperature,   "22.4 C / 72.3 F");
    g_fields.SetString(FieldId::EnvHumidity,      "47.2%");
    g_fields.SetString(FieldId::EnvPressure,      "101.3 kPa");
    g_fields.SetString(FieldId::EnvAirQuality,    "98");
    g_fields.SetString(FieldId::EnvCO2,           "412 PPM");
    g_fields.SetString(FieldId::EnvO2,            "20.9%");

    // === Environ — Decks ===
    g_fields.SetString(FieldId::Deck1Bridge,      "22.1 C  NOMINAL");
    g_fields.SetString(FieldId::Deck2Officers,    "22.3 C  NOMINAL");
    g_fields.SetString(FieldId::Deck3Crew,        "22.0 C  NOMINAL");
    g_fields.SetString(FieldId::Deck5Science,     "21.8 C  NOMINAL");
    g_fields.SetString(FieldId::Deck7Engineering, "24.7 C  ELEVATED");
    g_fields.SetString(FieldId::Deck10Cargo,      "19.2 C  NOMINAL");

    // === Power — Generation ===
    g_fields.SetString(FieldId::GenWarpCore,      "1,247.3 MW");
    g_fields.SetString(FieldId::GenImpulse,       "312.8 MW");
    g_fields.SetString(FieldId::GenSolar,         "47.2 MW");
    g_fields.SetString(FieldId::GenAuxFusion,     "89.1 MW");
    g_fields.SetString(FieldId::GenTotal,         "1,696.4 MW");

    // === Power — Consumption ===
    g_fields.SetString(FieldId::ConPropulsion,    "847.2 MW");
    g_fields.SetString(FieldId::ConShields,       "198.4 MW");
    g_fields.SetString(FieldId::ConLifeSupport,   "112.7 MW");
    g_fields.SetString(FieldId::ConSensors,       "78.3 MW");
    g_fields.SetString(FieldId::ConComputers,     "64.1 MW");
    g_fields.SetString(FieldId::ConWeapons,       "0.0 MW  STANDBY");
    g_fields.SetString(FieldId::ConTotal,         "1,300.7 MW");

    // === Power — Reserves ===
    g_fields.SetString(FieldId::BatteryA,         "98.2%");
    g_fields.SetString(FieldId::BatteryB,         "97.8%");
    g_fields.SetString(FieldId::Emergency,        "100%");
    g_fields.SetString(FieldId::Surplus,          "395.7 MW");

    // === Radio — Channels ===
    g_fields.SetString(FieldId::Ch1Subspace,      "47.3 GHz  ACTIVE");
    g_fields.SetString(FieldId::Ch2Emergency,     "121.5 MHz  MONITORING");
    g_fields.SetString(FieldId::Ch3Tactical,      "89.7 GHz  STANDBY");
    g_fields.SetString(FieldId::Ch4Science,       "22.1 GHz  ACTIVE");
    g_fields.SetString(FieldId::Ch5Command,       "63.4 GHz  ENCRYPTED");
    g_fields.SetString(FieldId::Ch6Diplomatic,    "41.8 GHz  STANDBY");

    // === Radio — Signal Status ===
    g_fields.SetString(FieldId::SubspaceBandwidth,"47.3 GHz");
    g_fields.SetString(FieldId::SignalStrength,   "-42 dBm");
    g_fields.SetString(FieldId::NoiseFloor,       "-110 dBm");
    g_fields.SetString(FieldId::SNR,              "68 dB");
    g_fields.SetString(FieldId::AntennaArray,     "ALIGNED");
    g_fields.SetString(FieldId::Range,            "4.7 LY");

    // === Graph buffers ===
    float ft = (float)t;

    // Helper lambda to populate a graph buffer from a GraphDef
    auto populateGraph = [ft](const GraphDef& gd) {
        int idx = static_cast<int>(gd.id);
        for (int i = 0; i < kGraphSamples; i++) {
            float x = (float)i / (float)(kGraphSamples - 1) * kGraphXMax;
            g_graphBufs[idx].xs[i] = x;
            for (int l = 0; l < 2; l++) {
                const WaveParams& w = gd.lines[l].wave;
                g_graphBufs[idx].ys[l][i] = w.base
                    + sinf(x * w.xf1 + ft * w.tf1 + w.p1) * w.a1
                    + sinf(x * w.xf2 + ft * w.tf2 + w.p2) * w.a2;
            }
        }
    };

    // Populate graphs from each view's content
    for (const auto& gd : kTacticalContent.graphs) populateGraph(gd);
    for (const auto& gd : kEnvironContent.graphs)  populateGraph(gd);
    for (const auto& gd : kPowerContent.graphs)    populateGraph(gd);
    for (const auto& gd : kRadioContent.graphs)    populateGraph(gd);
}
