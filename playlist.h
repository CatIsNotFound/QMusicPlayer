
#ifndef QMUSICPLAYER_PLAYLIST_H
#define QMUSICPLAYER_PLAYLIST_H
#include <QString>
#include <QList>
class Playlist {

public:
    Playlist() = default;
    Playlist(const Playlist& playlist);

    // 歌曲结构体
    struct Song {
        QString title;
        QString artist;
        QString album;
        QString path;
    };

    const uint addSongToList(const Song& song);
    void removeSongByIndex(const uint index);
    void removeSongByPath(const QString& path);
    void clearPlaylist();
    const QList<Song> getPlayList() const;
    const Song getSongByIndex(const uint index);
    const Song getSongByPath(const QString& path);
    const uint findSongByPath(const QString& path);
    const uint getSongCount() const;

private:
    QList<Song> play_list;
};


#endif //QMUSICPLAYER_PLAYLIST_H
