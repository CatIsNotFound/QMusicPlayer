
#ifndef MUSICPLAYER_MUSICPLAYER_H
#define MUSICPLAYER_MUSICPLAYER_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QPointer>
#include <QLabel>
#include <QTableView>
#include <QStandardItemModel>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QDesktopServices>
#include <QThread>
#include <QMimeData>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/flacfile.h>

#include "Player.h"
#include "AboutApp.h"
#include "playlist.h"
#include "AppConfig.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MusicPlayer; }
QT_END_NAMESPACE

class ImportSongsTask : public QThread {
    Q_OBJECT
public:
    ImportSongsTask() : directory_name(QDir::homePath()) {}
    ImportSongsTask(const QString& dir) : directory_name(dir) {}

    void setDirectory(const QString& dir) { directory_name = dir; }

    void run() override;
    const Playlist& getImportedPlaylist();
    void clearImportedPlaylist();

signals:
    void finishedWork(int success_count);
    void addSong(const Playlist::Song& song);

private:
    QString directory_name;
    Playlist imported_play_list;
};

class AddMultiSongsTask : public QThread {
    Q_OBJECT
public:
    AddMultiSongsTask();
    AddMultiSongsTask(QList<QString> &urls);

    void setTaskList(const QList<QString> &urls);
    void clearTaskList();
    void run() override;
signals:
    void addSongToList(const Playlist::Song& new_song);
    void finishedWork();
private:
    QList<QString> path_list;
    Playlist added_playlist;
};

class MusicPlayer : public QMainWindow {
Q_OBJECT

public:
    explicit MusicPlayer(QWidget *parent = nullptr);

    ~MusicPlayer() override;

    void setupConnection();
    void setupList();
    void setupOptions();
    void setupOthers();
    void saveOptions();
    void updateSelectionCount();
    bool addFileToPlaylist(const QString& url);
    void addSongToPlaylist(const Playlist::Song& song);

public slots:
    void nextSong();
    void lastSong();

protected:
    void firstSong();
    void replayCurrentSong();
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent* event) override;

protected slots:
    void openFileAndPlayFile();
    void playPause();
    void getSongInfo(const QString& url);
    void playSongFromList();
    void importSongToList();
    void importSongDirectoryToList();
    void removeSongFromList();
    void clearPlayList();
    void openGithub();
    void showReadMe();
    void showVersion();

private:
    Ui::MusicPlayer *ui;
    QPointer<QSystemTrayIcon> trayIcon;
    QPointer<QMenu> trayMenu, listMenu;
    QPointer<QAction> action_ShowUI, action_Quit;
    QPointer<QStandardItemModel> model;
    QPointer<QTableView> table;
    QPointer<QLabel> status_list, status_playing;
    Playlist playlist;
    qint64 current_playing{-1};
    QPointer<ImportSongsTask> import_task;
    QList<ImportSongsTask*> multi_import_task;
    QPointer<AddMultiSongsTask> add_songs_task;
    AppOptions app_options{};
    bool is_not_closed{true};
};

#endif //MUSICPLAYER_MUSICPLAYER_H
