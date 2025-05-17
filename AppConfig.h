#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QFile>
#include <nlohmann/json.hpp>
#include "Player.h"


struct AppOptions {
    // Player
    PlayLoopMode playLoopMode;
    int volume;
    bool mutedVolume;
    // Playlist
    QList<QString> path_list;
    // General
    int window_width;
    int window_height;
    int window_state;
};

namespace Apps {
    inline void saveAppOptions(const AppOptions& options) {
        QFile file("app.conf");
        if (file.open(QIODevice::WriteOnly)) {
            nlohmann::json json;
            // Play loop mode
            if (options.playLoopMode == PlayLoopMode::NoLoop) {
                json["playloop"] = "Once";
            } else if (options.playLoopMode == PlayLoopMode::SingleSongLoop) {
                json["playloop"] = "Single";
            } else if (options.playLoopMode == PlayLoopMode::ListLoop) {
                json["playloop"] = "List";
            }
            json["volume"] = options.volume;
            json["is_muted"] = options.mutedVolume;
            // Path list
            nlohmann::json playlist = nlohmann::json::array();
            for (const QString& i : options.path_list) {
                playlist.emplace_back(i.toUtf8().toStdString().data());
            }
            json["urls"] = playlist;
            nlohmann::json window;
            window["width"] = options.window_width;
            window["height"] = options.window_height;
            window["state"] = options.window_state;
            json["window"] = window;
            file.write(json.dump(4).data());
            file.close();
        }
    }

    inline bool loadAppOptions(AppOptions& options) {
        QFile file("app.conf");
        if (file.open(QIODevice::ReadOnly)) {
            auto json = nlohmann::json::parse(file.readAll().toStdString());
            // Play loop mode
            std::string mode = json["playloop"];
            if (mode == "Once") options.playLoopMode = PlayLoopMode::NoLoop;
            else if (mode == "Single") options.playLoopMode = PlayLoopMode::SingleSongLoop;
            else if (mode == "List") options.playLoopMode = PlayLoopMode::ListLoop;
            options.volume = json["volume"];
            options.mutedVolume = json["is_muted"];
            // Path list
            std::vector<std::string> playlist = json["urls"];
            for (std::string& i : playlist) {
                options.path_list.emplace_back(i.data());
            }
            file.close();
            // Window
            nlohmann::json window = json["window"];
            options.window_width = window["width"];
            options.window_height = window["height"];
            options.window_state = window["state"];
            return true;
        }
        return false;
    }
}

#endif // APPCONFIG_H
