
#ifndef MUSICPLAYER_MUSICPLAYER_H
#define MUSICPLAYER_MUSICPLAYER_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QPointer>
#include <QTableView>
#include <QStandardItemModel>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <taglib/fileref.h>
#include <taglib/tag.h>

#include "Player.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MusicPlayer; }
QT_END_NAMESPACE

class MusicPlayer : public QMainWindow {
Q_OBJECT

public:
    explicit MusicPlayer(QWidget *parent = nullptr);

    ~MusicPlayer() override;

    void setupConnection();
    void setupList();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

protected slots:
    void openFileAndPlayFile();
    void playPause();

private:
    Ui::MusicPlayer *ui;
    QPointer<QSystemTrayIcon> trayIcon;
    QPointer<QMenu> trayMenu;
    QPointer<QAction> action_ShowUI, action_Quit;
    QPointer<QStandardItemModel> model;
    QPointer<QTableView> table;
    QStandardItem* col_path;
    QStandardItem* col_songName;
    QStandardItem* col_artist;
};


#endif //MUSICPLAYER_MUSICPLAYER_H
