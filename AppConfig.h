#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QFile>
#include <nlohmann/json.hpp>
#include "Player.h"


struct AppOptions {
    // Player
    PlayLoopMode playLoopMode;
    int volume;
    bool muted_volume;
    bool auto_play_music;
    QString audio_output;
    bool auto_change_audio_output;
    bool auto_pause_song_when_audio_output_changed;
    // Playlist
    QList<QString> path_list;
    bool remember_playlist_id;
    uint playlist_id;
    // Notification
    bool close_window_notification;
    bool start_up_notification;
    bool change_song_notification;
    // Window
    int window_width;
    int window_height;
    int window_state;
    // Theme
    QString theme_path;
};

namespace Apps {
    inline void saveAppOptions(const AppOptions& options) {
        QFile file("app.conf");
        if (file.open(QIODevice::WriteOnly)) {
            nlohmann::json json, player;
            // Play loop mode
            if (options.playLoopMode == PlayLoopMode::NoLoop) {
                player["playloop"] = "Once";
            } else if (options.playLoopMode == PlayLoopMode::SingleSongLoop) {
                player["playloop"] = "Single";
            } else if (options.playLoopMode == PlayLoopMode::ListLoop) {
                player["playloop"] = "List";
            }
            player["volume"] = options.volume;
            player["is_muted"] = options.muted_volume;
            //player["playlist_id"] = options.playlist_id;
            player["auto_play_when_startup"] = options.auto_play_music;
            player["auto_change_audio_output"] = options.auto_change_audio_output;
            player["auto_paused_when_changed"] = options.auto_pause_song_when_audio_output_changed;
            json["player"] = player;
            // Playlist URL
            nlohmann::json playlist;
            nlohmann::json urls = nlohmann::json::array();
            for (const QString& i : options.path_list) {
                urls.emplace_back(i.toUtf8().toStdString().data());
            }
            playlist["urls"] = urls;
            playlist["remember_playlist_id"] = options.remember_playlist_id;
            playlist["playlist_id"] = options.playlist_id;
            json["playlist"] = playlist;
            // Notification
            nlohmann::json notification;
            notification["startup"] = options.start_up_notification;
            notification["closed"] = options.close_window_notification;
            notification["change_song"] = options.change_song_notification;
            json["notification"] = notification;
            nlohmann::json window;
            window["width"] = options.window_width;
            window["height"] = options.window_height;
            window["state"] = options.window_state;
            json["window"] = window;
            json["theme_path"] = options.theme_path.toStdString();
            file.write(json.dump(4).data());
            file.close();
        }
    }

    inline bool loadAppOptions(AppOptions& options) {
        QFile file("app.conf");
        if (file.open(QIODevice::ReadOnly)) {
            try {
                auto json = nlohmann::json::parse(file.readAll().toStdString());
                nlohmann::json player = json["player"];
                // Play loop mode
                std::string mode = player["playloop"];
                if (mode == "Once") options.playLoopMode = PlayLoopMode::NoLoop;
                else if (mode == "Single") options.playLoopMode = PlayLoopMode::SingleSongLoop;
                else if (mode == "List") options.playLoopMode = PlayLoopMode::ListLoop;
                options.volume = player["volume"];
                options.muted_volume = player["is_muted"];
                options.auto_play_music = player["auto_play_when_startup"];
                options.auto_change_audio_output = player["auto_change_audio_output"];
                options.auto_pause_song_when_audio_output_changed = player["auto_paused_when_changed"];
                // Path list
                nlohmann::json playlist = json["playlist"];
                options.playlist_id = playlist["playlist_id"];
                options.remember_playlist_id = playlist["remember_playlist_id"];
                std::vector<std::string> urls = playlist["urls"];
                for (std::string &i: urls) {
                    options.path_list.emplace_back(i.data());
                }
                // Notification
                nlohmann::json notification = json["notification"];
                options.start_up_notification = notification["startup"];
                options.close_window_notification = notification["closed"];
                options.change_song_notification = notification["change_song"];
                // Window
                nlohmann::json window = json["window"];
                options.window_width = window["width"];
                options.window_height = window["height"];
                options.window_state = window["state"];
                // Theme
                options.theme_path = QString::fromStdString(json["theme_path"]);
                file.close();
                return true;
            } catch (const std::exception& e) {
                file.close();
                qDebug().noquote() << "[Error] Can not load app options! Codes: " << e.what();
                return false;
            }
        }
        return false;
    }
}

#endif // APPCONFIG_H
