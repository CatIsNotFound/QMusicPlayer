
#include "MusicPlayer.h"
#include "ui_MusicPlayer.h"


MusicPlayer::MusicPlayer(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MusicPlayer) {
    ui->setupUi(this);
    // ui->playerWidget = new Player(ui->splitter);
    trayMenu = new QMenu(this);
    action_ShowUI = new QAction("显示主界面(&S)");
    action_Quit = new QAction("退出(&Q)");
    action_ShowUI->setIcon(QIcon(":/assets/app.svg"));
    action_Quit->setIcon(QIcon(":/assets/关闭_close-one.svg"));
    trayMenu->addAction(ui->action_Play_Pause);
    trayMenu->addAction(ui->action_StopSong);
    trayMenu->addSeparator();
    trayMenu->addAction(ui->action_LastSong);
    trayMenu->addAction(ui->action_NextSong);
    trayMenu->addSeparator();
    trayMenu->addAction(action_ShowUI);
    trayMenu->addAction(action_Quit);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QApplication::windowIcon());
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
    trayIcon->setToolTip(QApplication::applicationName());
    setupList();
    setupConnection();
    setupOthers();
    setWindowTitle(QApplication::applicationName());
}

MusicPlayer::~MusicPlayer() {
    delete ui;
}

void MusicPlayer::setupConnection() {
    // 系统后台
    connect(action_ShowUI, &QAction::triggered, [this] {
        setWindowState(Qt::WindowActive);
        show();
    });
    connect(action_Quit, &QAction::triggered, this, &QApplication::quit);
    connect(trayIcon, &QSystemTrayIcon::activated, this, [&] (QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            this->show();
            this->setWindowState(Qt::WindowActive);
        }
    });
    // 文件
    connect(ui->action_Open, &QAction::triggered, this, &MusicPlayer::openFileAndPlayFile);
    connect(ui->action_Exit, &QAction::triggered, this, &QApplication::quit);

    // 控制播放
    connect(ui->action_Play_Pause, &QAction::triggered, this, &MusicPlayer::playPause);
    connect(ui->action_StopSong, &QAction::triggered, ui->playerWidget, &Player::stop);
    connect(ui->action_Backward, &QAction::triggered, ui->playerWidget, &Player::backward3Sec);
    connect(ui->action_Forward, &QAction::triggered, ui->playerWidget, &Player::forward3Sec);
    connect(ui->action_NextSong, &QAction::triggered, this, []{});
    connect(ui->action_LastSong, &QAction::triggered, this, []{});
    connect(ui->action_VolumeUp, &QAction::triggered, [this]{
        ui->action_VolumeMute->setCheckable(false);
        ui->playerWidget->changeVolume(ui->playerWidget->volume() + 5);
    });
    connect(ui->action_VolumeDown, &QAction::triggered, [this] {
        ui->action_VolumeMute->setCheckable(false);
        ui->playerWidget->changeVolume(ui->playerWidget->volume() - 5);
    });
    connect(ui->action_VolumeMute, &QAction::triggered, [this] {
        ui->playerWidget->muteVolume();
        ui->action_VolumeMute->setChecked(ui->playerWidget->isMuted());
    });

    // 帮助
    connect(ui->action_Github, &QAction::triggered, this, &MusicPlayer::openGithub);
    connect(ui->action_Readme, &QAction::triggered, this, &MusicPlayer::showReadMe);
    connect(ui->action_About, &QAction::triggered, this, &MusicPlayer::showVersion);
    connect(ui->action_AboutQt, &QAction::triggered, this, &QApplication::aboutQt);

    // 播放器
    connect(ui->playerWidget, &Player::invalidMedia, this, [this](const QString& url) {
        QMessageBox::critical(this, "无法打开", QString("无法打开并播放文件 %1，可能此文件不是有效的可播放音乐文件！").arg(url));
    });
    connect(ui->playerWidget, &Player::songsChanged, this, &MusicPlayer::getSongInfo);
    connect(ui->playerWidget, &Player::playControlChanged, [this](const bool enabled) {
        ui->action_Backward->setEnabled(enabled);
        ui->action_Forward->setEnabled(enabled);
        ui->action_StopSong->setEnabled(enabled);
    });
    connect(ui->playerWidget, &Player::resetPlayer, [this] {
        setWindowTitle(QApplication::applicationName());
        trayIcon->setToolTip(QApplication::applicationName());
        status_playing->setText("当前无媒体播放");
    });
    connect(ui->playerWidget, &Player::nextSongRequest, this, &MusicPlayer::nextSong);
    connect(ui->playerWidget, &Player::lastSongRequest, this, &MusicPlayer::lastSong);
    connect(ui->action_NextSong, &QAction::triggered, this, &MusicPlayer::nextSong);
    connect(ui->action_LastSong, &QAction::triggered, this, &MusicPlayer::lastSong);

    // 播放列表
    connect(ui->action_PlaySongImmediately, &QAction::triggered, this, &MusicPlayer::playSongFromList);
    connect(ui->action_ImportSong, &QAction::triggered, this, &MusicPlayer::importSongToList);
    connect(ui->action_ImportMoreSongsToList, &QAction::triggered, this, &MusicPlayer::importSongDirectoryToList);
    connect(ui->action_DeleteSong, &QAction::triggered, this, &MusicPlayer::removeSongFromList);
    connect(ui->action_ClearList, &QAction::triggered, this, &MusicPlayer::clearPlayList);

}

void MusicPlayer::setupList() {
    model = new QStandardItemModel;
    table = new QTableView();
    QStringList headers;
    headers << "歌曲名" << "歌手" << "专辑" << "文件路径";
    model->setHorizontalHeaderLabels(headers);
    table->setModel(model);
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    table->setSelectionMode(QTableView::ExtendedSelection);
    table->setSelectionBehavior(QTableView::SelectRows);
    table->setEditTriggers(QTableView::NoEditTriggers);
    connect(table, &QTableView::doubleClicked, this, &MusicPlayer::playSongFromList);
    connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MusicPlayer::updateSelectionCount);
    listMenu = new QMenu();
    listMenu->addAction(ui->action_PlaySongImmediately);
    listMenu->addAction(ui->action_DeleteSong);
    listMenu->addSeparator();
    listMenu->addAction(ui->action_ImportSong);
    listMenu->addAction(ui->action_ImportMoreSongsToList);
    listMenu->addAction(ui->action_ClearList);
    connect(table, &QTableView::customContextMenuRequested, [&] {
        listMenu->move(QCursor::pos());
        listMenu->show();
    });
    ui->tabWidget->clear();
    ui->tabWidget->addTab(table, "播放列表");
}

void MusicPlayer::setupOthers() {
    status_list = new QLabel("当前播放列表共 0 项");
    status_playing = new QLabel("当前无媒体播放");
    ui->statusbar->addWidget(status_list, 1);
    ui->statusbar->addWidget(status_playing);
}

void MusicPlayer::updateSelectionCount() {
    // int idx = 0;
    int count = table->selectionModel()->selectedRows().size();
    if (count) {
        status_list->setText(QString("已选择 %1 项").arg(count));
    } else {
        status_list->setText(QString("当前播放列表共 %1 项").arg(model->rowCount()));
    }
}

void MusicPlayer::nextSong() {
    if (current_playing != -1) {
        model->item(current_playing)->setIcon(QIcon());
    }
    if (model->rowCount()) {
        if (++current_playing >= model->rowCount()) {
            current_playing = 0;
        }
        model->item(current_playing)->setIcon(QIcon(":/assets/playSong.svg"));
        ui->playerWidget->playSongImmediately(model->item(current_playing, 3)->text());
    }
}

void MusicPlayer::lastSong() {
    if (current_playing != -1) {
        model->item(current_playing)->setIcon(QIcon());
    }
    if (model->rowCount()) {
        if (--current_playing < 0) {
            current_playing = model->rowCount() - 1;
        }
        model->item(current_playing)->setIcon(QIcon(":/assets/playSong.svg"));
        ui->playerWidget->playSongImmediately(model->item(current_playing, 3)->text());
    }
}

// 仅在删除时才调用
void MusicPlayer::firstSong() {
    if (model->rowCount() == 0) return;
    current_playing = 0;
    ui->playerWidget->playSongImmediately(model->item(0, 3)->text());
    model->item(0, 0)->setIcon(QIcon(":/assets/playSong.svg"));
}

// 仅在删除时调用
void MusicPlayer::replayCurrentSong() {
    ui->playerWidget->playSongImmediately(model->item(current_playing, 3)->text());
    model->item(current_playing)->setIcon(QIcon(":/assets/playSong.svg"));
}

void MusicPlayer::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    ui->splitter->setGeometry(QRect(0, 0, ui->centralwidget->width(), ui->centralwidget->height()));
}

void MusicPlayer::closeEvent(QCloseEvent *event) {
    event->ignore();
    // trayIcon->showMessage(QApplication::applicationName(), "请注意：程序仍在运行，不要忘记从托盘图标中打开！");
    hide();
}

void MusicPlayer::openFileAndPlayFile() {
    QFileDialog fileDialog;
    QString fileName = fileDialog.getOpenFileName(this, "打开...", QDir::homePath(), "MP3文件(*.mp3);;FLAC文件(*.flac);;音频文件(*.wav);;所有文件(*.*)");
    if (fileName.isEmpty()) return;
    qDebug() << "Open:" << fileName;
    ui->playerWidget->playSongImmediately(fileName);
    if (current_playing != -1) model->item(current_playing)->setIcon(QIcon());
    current_playing = -1;
}

void MusicPlayer::playPause() {
    if (ui->playerWidget->isPlayingSong()) {
        ui->playerWidget->pause();
    } else {
        ui->playerWidget->play();
    }
}

void MusicPlayer::getSongInfo(const QString &url) {
    TagLib::FileRef f(url.toStdString().data());
    if (!f.isNull() && f.tag()) {
        qInfo() << "[Songs Detail]";
        qInfo() << "Title: " << f.tag()->title().toCString(true);
        qInfo() << "Artist: " << f.tag()->artist().toCString(true);
        qInfo() << "Album: " << f.tag()->album().toCString(true);
    }
    status_playing->setText(url.right(url.size()- url.lastIndexOf('/') - 1));
    setWindowTitle(QString("%1 - %2").arg(url.right(url.size()- url.lastIndexOf('/') - 1)).arg(QApplication::applicationName()));
    trayIcon->setToolTip(QString("%1 - %2").arg(url.right(url.size()- url.lastIndexOf('/') - 1)).arg(QApplication::applicationName()));
}

void MusicPlayer::playSongFromList() {
    auto idx = table->currentIndex().row();
    if (idx < 0) return;
    ui->playerWidget->playSongImmediately(model->item(idx, 3)->text());
    if (current_playing != -1) {
        model->item(current_playing)->setIcon(QIcon());
    }
    current_playing = idx;
    model->item(current_playing)->setIcon(QIcon(":/assets/playSong.svg"));
}

void MusicPlayer::importSongToList() {
    auto file_name = QFileDialog::getOpenFileName(this, "导入歌曲到播放列表", QDir::homePath(), "MP3文件(*.mp3);;FLAC文件(*.flac);;音频文件(*.wav);;所有文件(*.*)");
    if (file_name.isEmpty()) return;
    qInfo() << "Import to list: " << file_name;
    TagLib::FileRef f(file_name.toStdString().data());
    Playlist::Song temp_song;
    if (!f.isNull()) {
        if (f.tag()) {
            temp_song.title = QString::fromStdString(f.tag()->title().toCString(true));
            temp_song.artist = QString::fromStdString(f.tag()->artist().toCString(true));
            temp_song.album = QString::fromStdString(f.tag()->album().toCString(true));
            temp_song.path = file_name;
        } else {
            temp_song.title = file_name.right(file_name.lastIndexOf('/'));
            temp_song.artist = "";
            temp_song.album = "";
            temp_song.path = file_name;
        }
    }
    if (temp_song.title.isEmpty()) {
        temp_song.title = file_name.right(file_name.size()- file_name.lastIndexOf('/') - 1);
        temp_song.path = file_name;
    }
    uint idx = playlist.addSongToList(temp_song);
    if (idx != UINT16_MAX) {
        QList<QStandardItem*> item_list;
        item_list.emplace_back(new QStandardItem(temp_song.title));
        item_list.emplace_back(new QStandardItem(temp_song.artist));
        item_list.emplace_back(new QStandardItem(temp_song.album));
        item_list.emplace_back(new QStandardItem(temp_song.path));
        model->appendRow(item_list);
    }
    status_list->setText(QString("当前播放列表共 %1 项").arg(model->rowCount()));
}

void MusicPlayer::importSongDirectoryToList() {
    auto dir_name = QFileDialog::getExistingDirectory(this, "导入歌曲目录到列表", QDir::homePath());
    if (dir_name.isEmpty()) return;
    qInfo() << "Import Directory: " << dir_name;
    if (!import_task) import_task = new ImportSongsTask();
    import_task->setDirectory(dir_name);
    import_task->run();
}

void MusicPlayer::removeSongFromList() {
    int row = table->currentIndex().row();
    qDebug() << "Removed row: " << row;
    if (row < 0) return;
    // 获取当前播放的路径
    QString c_playing;
    if (current_playing != -1) {
        c_playing = model->item(current_playing, 3)->text();
    }
    // 获取当前选择的行
    auto selection = table->selectionModel()->selectedRows();
    QList<uint> delete_row_list;    // 用于存储要删除的行
    for (auto& s : selection) {
        delete_row_list.emplace_back(s.row());
    }
    std::sort(delete_row_list.begin(), delete_row_list.end(), std::greater<uint>());

    bool is_deleted = false; // 检查当前播放歌曲是否删除

    for (uint& d : delete_row_list) {
        if (current_playing == d) is_deleted = true;
        playlist.removeSongByPath(model->item(d, 3)->text());
        model->removeRow(d);
    }

    if (is_deleted) {
        // 若此索引已是最后一首，选择第一首歌曲；
        // 若后面仍有歌曲，则索引不变；
        // 若列表为空，直接停止播放
        if (current_playing > model->rowCount()) {
            firstSong();
        } else if (current_playing > -1 && current_playing < model->rowCount()) {
            replayCurrentSong();
        } else {
            current_playing = -1;
            ui->playerWidget->dropOutSong();
        }
    } else {
        if (!c_playing.isEmpty()) {
            uint t = playlist.findSongByPath(c_playing); // 获取索引
            if (t != UINT16_MAX) { current_playing = t; }
            else current_playing = -1;
        }
    }

    qDebug() << "Row count:" << model->rowCount();
    qDebug() << "Song count:" << playlist.getSongCount();
    status_list->setText(QString("当前播放列表共 %1 项").arg(model->rowCount()));
}

void MusicPlayer::clearPlayList() {
    if (model->rowCount() != 0) {
        auto res = QMessageBox::question(this, "清空项目", "是否确认清空该列表下的所有播放歌曲？");
        if (res == QMessageBox::Yes) {
            model->removeRows(0, model->rowCount());
            playlist.clearPlaylist();
            current_playing = -1;
            ui->playerWidget->dropOutSong();
        }
    }
    status_list->setText(QString("当前播放列表共 %1 项").arg(model->rowCount()));
}

void MusicPlayer::openGithub() {
    QDesktopServices::openUrl(QUrl("https://github.com/CatIsNotFound/QMusicPlayer"));
}

void MusicPlayer::showReadMe() {
    QDesktopServices::openUrl(QUrl("https://github.com/CatIsNotFound/QMusicPlayer/blob/master/README.md"));
}

void MusicPlayer::showVersion() {
    AboutApp aboutApp;
    aboutApp.exec();
}

void ImportSongsTask::run() {
    QThread::run();
    QDir dir(directory_name);
    auto songs_list = dir.entryList(QDir::Files);
    for (auto& song : songs_list) {
        Playlist::Song temp_song;
        // 筛选
        if (song.endsWith("mp3") || song.endsWith("flac") || song.endsWith("wav")) {
            qInfo().noquote() << song << ": ";
            TagLib::FileRef mp3File(song.toStdString().data());
            if (!mp3File.isNull() && mp3File.tag()) {
                TagLib::Tag *tag = mp3File.tag();
                qInfo() << "MP3 元数据:" ;
                qInfo() << "标题: " << tag->title().toCString();
                qInfo() << "艺术家: " << tag->artist().toCString();
                qInfo() << "专辑: " << tag->album().toCString();
            }
            TagLib::FLAC::File flacFile(song.toStdString().data());
            if (flacFile.isValid()) {
                qInfo() << "\nFLAC 元数据:";
                TagLib::Tag *tag = flacFile.tag();
                qInfo() << "标题: " << tag->title().toCString();
                qInfo() << "艺术家: " << tag->artist().toCString();
                qInfo() << "专辑: " << tag->album().toCString();
            }
        }
    }
}
