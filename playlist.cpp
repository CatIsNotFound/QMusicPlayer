
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

bool Playlist::parseSongFromFileName(const QString &path, Song &song) {
    QFileInfo file(path);
    // 判断是否是文件
    if (!file.isFile()) return false;
    QByteArray codec = file.absoluteFilePath().toLocal8Bit();
    // 解析音乐文件
    qDebug() << "[Debug] Parsed" << codec.toStdString().c_str();
    TagLib::FileRef fileRef(codec.toStdString().c_str());
    // 文件无法解析！
    if (!fileRef.file()) return false;
    TagLib::Tag *music_tag = fileRef.tag();     // 获取音乐文件下的标签
    Song temp_song;                             // 用于将解析结果存入此结构体中
    temp_song.path = path;
    temp_song.title = file.fileName();          // 暂时以文件名命名标题
    // 若无法解析标签
    if (!music_tag) {
        qInfo() << "[Error] Can not parse the file" << path << "!";
        temp_song.album = "未知专辑";
        temp_song.artist = "未知歌手";
        play_list.emplace_back(temp_song);
        song = temp_song;
        return true;
    }
    QString m_title = music_tag->title().toCString(true);
    // 若此音乐文件有标题
    if (!m_title.isEmpty()) {
        temp_song.title = m_title;
        QString artist = music_tag->artist().toCString(true);
        QString album = music_tag->album().toCString(true);
        temp_song.artist = (artist.isEmpty() ? "未知歌手" : artist);
        temp_song.album = (album.isEmpty() ? "未知专辑" : album);
    }
    // 已成功解析
    play_list.emplace_back(temp_song);
    song = temp_song;
    return true;
}
