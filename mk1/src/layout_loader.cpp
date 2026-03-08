#include "layout_loader.h"
#include <toml++/toml.hpp>
#include <iostream>
#include <fstream>

// Helper to parse orientation from TOML table
static OrientationDef ParseOrientation(const toml::table& orientTable) {
    OrientationDef orient;

    if (auto h = orientTable["horizontal"].value<std::string>()) {
        if (*h == "left") orient.horizontal = HOrientation::Left;
        else if (*h == "right") orient.horizontal = HOrientation::Right;
    }

    if (auto v = orientTable["vertical"].value<std::string>()) {
        if (*v == "top") orient.vertical = VOrientation::Top;
        else if (*v == "bottom") orient.vertical = VOrientation::Bottom;
    }

    return orient;
}

// Helper to parse panel sizing from TOML
static PanelSizing ParseSizing(const std::string& sizingStr) {
    if (sizingStr == "fixed") return PanelSizing::Fixed;
    return PanelSizing::Auto;
}

LayoutLoadResult LoadLayoutFromFile(const std::string& path) {
    LayoutLoadResult result;

    try {
        // Parse TOML file
        toml::table tbl = toml::parse_file(path);

        // Parse screen section
        if (auto screen = tbl["screen"].as_table()) {
            if (auto title = (*screen)["title"].value<std::string>()) {
                result.model.title = *title;
            }
        }

        // Parse sockets array
        if (auto socketsArray = tbl["sockets"].as_array()) {
            for (const auto& elem : *socketsArray) {
                if (auto sockTbl = elem.as_table()) {
                    SocketDef sock;
                    if (auto id = (*sockTbl)["id"].value<std::string>()) {
                        sock.id = *id;
                    }
                    if (auto path = (*sockTbl)["path"].value<std::string>()) {
                        sock.path = *path;
                    }
                    result.model.sockets.push_back(sock);
                }
            }
        }

        // Parse panels array
        if (auto panelsArray = tbl["panels"].as_array()) {
            for (const auto& elem : *panelsArray) {
                if (auto panelTbl = elem.as_table()) {
                    PanelDef panel;

                    if (auto id = (*panelTbl)["id"].value<std::string>()) {
                        panel.id = *id;
                    }
                    if (auto title = (*panelTbl)["title"].value<std::string>()) {
                        panel.title = *title;
                    }
                    if (auto socket = (*panelTbl)["socket"].value<std::string>()) {
                        panel.socket_id = *socket;
                    }

                    // Parse orientation
                    if (auto orientTbl = (*panelTbl)["orientation"].as_table()) {
                        panel.orientation = ParseOrientation(*orientTbl);
                    }

                    // Parse sizing
                    if (auto sizing = (*panelTbl)["sizing"].value<std::string>()) {
                        panel.sizing = ParseSizing(*sizing);
                        if (panel.sizing == PanelSizing::Fixed) {
                            if (auto height = (*panelTbl)["height"].value<double>()) {
                                panel.fixed_height = static_cast<float>(*height);
                            }
                        }
                    }

                    // TODO: Parse views array in future phases

                    result.model.panels.push_back(panel);
                }
            }
        }

        result.success = true;
    } catch (const toml::parse_error& err) {
        result.success = false;
        result.error_message = std::string("TOML parse error: ") + err.what();
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Error loading layout: ") + e.what();
    }

    return result;
}

void PrintScreenModel(const ScreenModel& model) {
    std::cout << "=== Screen Model ===" << std::endl;
    std::cout << "Title: " << model.title << std::endl;
    std::cout << std::endl;

    std::cout << "Sockets (" << model.sockets.size() << "):" << std::endl;
    for (const auto& sock : model.sockets) {
        std::cout << "  - id: " << sock.id << ", path: " << sock.path << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Panels (" << model.panels.size() << "):" << std::endl;
    for (const auto& panel : model.panels) {
        std::cout << "  - id: " << panel.id << std::endl;
        std::cout << "    title: " << panel.title << std::endl;
        std::cout << "    socket: " << panel.socket_id << std::endl;

        std::string hOrient = (panel.orientation.horizontal == HOrientation::Left) ? "left" : "right";
        std::string vOrient = (panel.orientation.vertical == VOrientation::Top) ? "top" : "bottom";
        std::cout << "    orientation: horizontal=" << hOrient << ", vertical=" << vOrient << std::endl;

        std::string sizing = (panel.sizing == PanelSizing::Auto) ? "auto" : "fixed";
        std::cout << "    sizing: " << sizing;
        if (panel.sizing == PanelSizing::Fixed) {
            std::cout << " (height=" << panel.fixed_height << ")";
        }
        std::cout << std::endl;

        std::cout << "    views: " << panel.views.size() << std::endl;
    }
    std::cout << "===================" << std::endl;
}
