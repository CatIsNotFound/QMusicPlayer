
#include "playlist.h"

Playlist::Playlist(const Playlist &playlist) {}

const uint Playlist::addSongToList(const Playlist::Song &song) {
    auto pos = findSongByPath(song.path);
    if (pos == UINT16_MAX) {
        play_list.emplace_back(song);
        return play_list.size() - 1;
    }
    return UINT16_MAX;
}

void Playlist::removeSongByIndex(const uint index) {
    if (index >= play_list.size() - 1)
        play_list.erase(play_list.begin() + index);
}

void Playlist::removeSongByPath(const QString &path) {
    uint idx = 0;
    for (auto& song : play_list) {
        if (!song.path.compare(path)) {
            play_list.removeAt(idx);
            break;
        }
        idx++;
    }
}

void Playlist::clearPlaylist() {
    play_list.clear();
}

const uint Playlist::findSongByPath(const QString &path) {
    uint idx = 0;
    for (auto& song: play_list) {
        if (!song.path.compare(path)) {
            return idx;
        }
        idx += 1;
    }
    return UINT16_MAX;  // 特殊值：找不到路径时输出
}

const uint Playlist::getSongCount() const {
    return play_list.size();
}

const QList<Playlist::Song> Playlist::getPlayList() const {
    return play_list;
}

const Playlist::Song Playlist::getSongByIndex(const uint index) {
    if (index >= play_list.size()) return Song();
    return play_list.at(index);
}

const Playlist::Song Playlist::getSongByPath(const QString &path) {
    for (auto& song : play_list) {
        if (!song.path.compare(path)) {
            return song;
        }
    }
    return Song();
}
