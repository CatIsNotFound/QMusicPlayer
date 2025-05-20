
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
    setupOptions();
    setWindowTitle(QApplication::applicationName());
}

MusicPlayer::~MusicPlayer() {
    delete ui;
    for (auto& task : multi_import_task) {
        delete task;
    }
}

void MusicPlayer::setupConnection() {
    // 系统后台
    connect(action_ShowUI, &QAction::triggered, [this] {
        setWindowState(Qt::WindowActive);
        show();
    });
    connect(action_Quit, &QAction::triggered, [this] { is_not_closed = false; QApplication::quit(); });
    connect(trayIcon, &QSystemTrayIcon::activated, this, [&] (QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            this->show();
            this->setWindowState(Qt::WindowActive);
        }
    });
    connect(ui->playerWidget, &Player::deviceChanged, [&] {
        // qDebug() << "Device changed!";
        if (options_window) {
            auto audio_list = ui->playerWidget->getAllAudioDevice();
            options_window->setAudioDeviceList(audio_list);
        }
    });

    // 启动后自动播放或加载
    auto_play_timer.setInterval(1000);
    connect(&auto_play_timer, &QTimer::timeout, [this] {
        if (model->rowCount() == 0) {
            auto_play_timer.stop();
            return;
        }
        if (app_options.auto_play_music) {
            if (current_playing == -1) {
                current_playing = 0;
            }
            model->item(current_playing)->setIcon(QIcon(":/assets/playSong.svg"));
            getSongInfo(model->item(current_playing, 3)->text());
            ui->playerWidget->playSongImmediately(model->item(current_playing, 3)->text());
        } else if (app_options.remember_playlist_id) {
            if (current_playing > -1) {
                ui->playerWidget->setSong(model->item(current_playing, 3)->text());
                model->item(current_playing)->setIcon(QIcon(":/assets/playSong.svg"));
                getSongInfo(model->item(current_playing, 3)->text());
            }
        }
        auto_play_timer.stop();
    });
    auto_play_timer.start();

    // 文件
    connect(ui->action_Open, &QAction::triggered, this, &MusicPlayer::openFileAndPlayFile);
    connect(ui->action_Options, &QAction::triggered, this, &MusicPlayer::showOptions);
    connect(ui->action_Exit, &QAction::triggered, [this] { is_not_closed = false; QApplication::quit(); });

    // 控制播放
    connect(ui->action_Play_Pause, &QAction::triggered, this, &MusicPlayer::playPause);
    connect(ui->action_StopSong, &QAction::triggered, ui->playerWidget, &Player::stop);
    connect(ui->action_Backward, &QAction::triggered, ui->playerWidget, &Player::backward3Sec);
    connect(ui->action_Forward, &QAction::triggered, ui->playerWidget, &Player::forward3Sec);
    connect(ui->action_NextSong, &QAction::triggered, this, &MusicPlayer::nextSong);
    connect(ui->action_LastSong, &QAction::triggered, this, &MusicPlayer::lastSong);
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
        trayIcon->showMessage("无法播放", QString("文件 %1 无法播放，可能该文件路径无效或此文件不是有效的可播放文件！").arg(url), QSystemTrayIcon::Critical);
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
    connect(ui->playerWidget, &Player::muteVolumeChanged, [this](const bool& enabled) {
        ui->action_VolumeMute->setChecked(enabled);
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
    connect(ui->action_MoveUpSong, &QAction::triggered, this, &MusicPlayer::moveUpSong);
    connect(ui->action_MoveDownSong, &QAction::triggered, this, &MusicPlayer::moveDownSong);
    connect(ui->action_MoveTopSong, &QAction::triggered, this, &MusicPlayer::moveTopSong);
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
    connect(table, &QTableView::customContextMenuRequested, [&] {
        QPoint cur = QCursor::pos();
        QPoint real_pos = cur;
        int m_width = ui->menu_P->geometry().width();
        int m_height = ui->menu_P->geometry().height();

        QSize scr_size = QGuiApplication::primaryScreen()->size();

        int menu_max_X = scr_size.width() - m_width;
        int menu_max_Y = scr_size.height() - m_height;
        if (cur.x() >= menu_max_X) {
            real_pos.setX(menu_max_X);
        }
        if (cur.y() >= menu_max_Y) {
            real_pos.setY(menu_max_Y);
        }
        ui->menu_P->move(real_pos);
        ui->menu_P->show();
    });
    ui->tabWidget->clear();
    ui->tabWidget->addTab(table, QIcon(":/assets/音乐菜单_music-list.svg"), "播放列表");
}

void MusicPlayer::setupOptions()
{
    if (Apps::loadAppOptions(app_options)) {
        ui->playerWidget->setPlayLoopMode(app_options.playLoopMode);
        ui->playerWidget->changeVolume(app_options.volume);
        ui->playerWidget->setAutoChangeOutputSwitch(app_options.auto_change_audio_output);
        ui->playerWidget->setAutoPausedSwitch(app_options.auto_pause_song_when_audio_output_changed);
        if (app_options.muted_volume) ui->playerWidget->muteVolume();
        // playlist
        add_songs_task = new AddMultiSongsTask(app_options.path_list);
        connect(add_songs_task, &AddMultiSongsTask::addSongToList, [this] (const Playlist::Song& new_song) {
            uint idx = playlist.findSongByPath(new_song.path);
            if (idx == UINT16_MAX) {
                playlist.addSongToList(new_song);
                addSongToPlaylist(new_song);
            }
        });
        connect(add_songs_task, &AddMultiSongsTask::finishedWork, [this] {
            add_songs_task->clearTaskList();
            if (app_options.remember_playlist_id) {
                current_playing = app_options.playlist_id - 1;
                if (current_playing >= playlist.getSongCount()) {
                    current_playing = 0;
                }
            }
            updateSelectionCount();
        });
        add_songs_task->start();
        // Window
        if (app_options.window_state == 0) {
            setWindowState(Qt::WindowMinimized);
            if (app_options.start_up_notification)
                trayIcon->showMessage(QApplication::applicationName(), "请注意：软件已启动并最小化，请从任务栏中显示主界面！");
        } else if (app_options.window_state == 1)
            setWindowState(Qt::WindowNoState);
        else if (app_options.window_state == 2)
            setWindowState(Qt::WindowMaximized);
        if (app_options.window_state == 1) {
            resize(app_options.window_width, app_options.window_height);
        }
        // Theme
        QFile file(app_options.theme_path);
        if (file.open(QIODevice::ReadOnly)) {
            setStyleSheet(file.readAll());
            file.close();
        }
    } else {
        // 当没有配置文件时，自动设置默认配置
        app_options.window_state = 1;
        app_options.audio_output = "";
        app_options.auto_play_music = false;
        app_options.auto_change_audio_output = true;
        app_options.auto_pause_song_when_audio_output_changed = true;
        app_options.remember_playlist_id = true;
        app_options.close_window_notification = true;
        app_options.start_up_notification = true;
        app_options.change_song_notification = true;
        ui->playerWidget->setAutoChangeOutputSwitch(app_options.auto_change_audio_output);
        ui->playerWidget->setAutoPausedSwitch(app_options.auto_pause_song_when_audio_output_changed);
    }
}

void MusicPlayer::setupOthers() {
    status_list = new QLabel("当前播放列表共 0 项");
    status_playing = new QLabel("当前无媒体播放");
    ui->statusbar->addWidget(status_list, 1);
    ui->statusbar->addWidget(status_playing);
}

void MusicPlayer::saveOptions() {
    // Player
    app_options.playLoopMode = ui->playerWidget->getPlayLoopMode();
    app_options.volume = ui->playerWidget->volume();
    app_options.muted_volume = ui->playerWidget->isMuted();
    // Playlist
    app_options.path_list.clear();
    for (uint i = 0; i < playlist.getSongCount(); ++i) {
        app_options.path_list.emplace_back(playlist.getSongByIndex(i).path);
    }
    if (app_options.remember_playlist_id)
        app_options.playlist_id = current_playing + 1;
    else
        app_options.playlist_id = 0;
    // Theme
    // qDebug() << "Saved Theme Config:" << app_options.theme_path;
    // Window
    app_options.window_width = width();
    app_options.window_height = height();
    if (windowState() == Qt::WindowMinimized)
        app_options.window_state = 0;
    else if (windowState() == Qt::WindowNoState)
        app_options.window_state = 1;
    else if (windowState() == Qt::WindowMaximized)
        app_options.window_state = 2;
    Apps::saveAppOptions(app_options);
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

bool MusicPlayer::addFileToPlaylist(const QString &url) {
    uint idx = playlist.findSongByPath(url);
    if (idx == UINT16_MAX) {
        Playlist::Song temp_song;
        if (playlist.parseSongFromFileName(url, temp_song)) {
            addSongToPlaylist(temp_song);
            return true;
        } else {
            QMessageBox::critical(this, "导入失败", QString("无法导入文件 %1，此文件不是有效的可播放文件！").arg(url));
            return false;
        }
    }
    return false;
}

void MusicPlayer::addSongToPlaylist(const Playlist::Song &temp_song) {
    QList<QStandardItem*> item_list;
    item_list.emplace_back(new QStandardItem(temp_song.title));
    item_list.emplace_back(new QStandardItem(temp_song.artist));
    item_list.emplace_back(new QStandardItem(temp_song.album));
    item_list.emplace_back(new QStandardItem(temp_song.path));
    model->appendRow(item_list);
}

void MusicPlayer::swap_songs(const int row1, const int row2) {
    if (playlist.swap(row1, row2)) {
        QString temp_info;
        if (current_playing == row1) {
            current_playing = row2;
            QIcon icon = model->item(row1)->icon();
            model->item(row2)->setIcon(icon);
            model->item(row1)->setIcon(QIcon());
        } else if (current_playing == row2) {
            current_playing = row1;
            QIcon icon = model->item(row2)->icon();
            model->item(row1)->setIcon(icon);
            model->item(row2)->setIcon(QIcon());
        }
        for (int i = 0; i < 4; ++i) {
            temp_info = model->item(row1, i)->text();
            model->item(row1, i)->setText(model->item(row2, i)->text());
            model->item(row2, i)->setText(temp_info);
        }
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
        if (app_options.change_song_notification && (windowState() == Qt::WindowMinimized || !this->isVisible())) {
            trayIcon->showMessage(QString("现在播放：%1").arg(model->item(current_playing)->text()), QString("%1 - %2").arg(model->item(current_playing)->text())
                                                                            .arg(model->item(current_playing, 1)->text()));
        }
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
        if (app_options.change_song_notification && (windowState() == Qt::WindowMinimized || !this->isVisible())) {
            trayIcon->showMessage(QString("现在播放：%1").arg(model->item(current_playing)->text()), QString("%1 - %2").arg(model->item(current_playing)->text())
                    .arg(model->item(current_playing, 1)->text()));
        }
    }
}

void MusicPlayer::moveUpSong() {
    if (table->currentIndex().row() > 0)
        swap_songs(table->currentIndex().row(), table->currentIndex().row() - 1);
}

void MusicPlayer::moveDownSong() {
    if (table->currentIndex().row() < model->rowCount())
        swap_songs(table->currentIndex().row(), table->currentIndex().row() + 1);
}

void MusicPlayer::moveTopSong() {
    if (table->currentIndex().row() > 0)
        swap_songs(table->currentIndex().row(), 0);
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
    if (is_not_closed && app_options.close_window_notification) {
        trayIcon->showMessage(QApplication::applicationName(), "请注意：程序仍在运行，不要忘记从托盘图标中打开！", QSystemTrayIcon::Information, 5000);
    } else saveOptions();
    hide();
}

void MusicPlayer::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MusicPlayer::dropEvent(QDropEvent *event)
{
    auto url_list = event->mimeData()->urls();
    for (auto& url : url_list) {
        QString real_url = url.path();
        if (!QSysInfo::kernelType().compare("winnt")) {
            real_url = real_url.removeFirst();
        }
        QFileInfo file(real_url);
        if (file.isDir()) {
            ImportSongsTask* task = new ImportSongsTask(real_url);
            connect(task, &ImportSongsTask::finishedWork, [&](int success_cnt) {
                trayIcon->showMessage("导入成功", QString("已成功导入 %1 个文件").arg(success_cnt));
            });
            connect(task, &ImportSongsTask::addSong, [&] (const Playlist::Song& new_song) {
                uint idx = playlist.findSongByPath(new_song.path);
                if (idx == UINT16_MAX) {
                    playlist.addSongToList(new_song);
                    addSongToPlaylist(new_song);
                }
            });
            multi_import_task.emplace_back(task);
            task->start();
        }
        else if (file.isFile()) addFileToPlaylist(real_url);
    }
    status_list->setText(QString("当前播放列表共 %1 项").arg(model->rowCount()));
}


void MusicPlayer::openFileAndPlayFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "打开...", QDir::homePath(), "MP3文件(*.mp3);;FLAC文件(*.flac);;OGG文件(*.ogg);;音频文件(*.wav);;所有文件(*.*)");
    if (fileName.isEmpty()) return;
    qDebug() << "Open:" << fileName;
    ui->playerWidget->playSongImmediately(fileName);
    if (current_playing != -1) model->item(current_playing)->setIcon(QIcon());
    current_playing = -1;
}

void MusicPlayer::showOptions() {
    if (!options_window) {
        options_window = new Options(this);
        connect(options_window, &Options::ConfigSaved, [this] (const AppOptions& new_options) {
             app_options.auto_play_music = new_options.auto_play_music;
             app_options.auto_change_audio_output = new_options.auto_change_audio_output;
             app_options.remember_playlist_id = new_options.remember_playlist_id;
             app_options.auto_pause_song_when_audio_output_changed = new_options.auto_pause_song_when_audio_output_changed;
             ui->playerWidget->setAutoChangeOutputSwitch(new_options.auto_change_audio_output);
             ui->playerWidget->setAutoPausedSwitch(new_options.auto_pause_song_when_audio_output_changed);

             app_options.start_up_notification = new_options.start_up_notification;
             app_options.close_window_notification = new_options.close_window_notification;
             app_options.change_song_notification = new_options.change_song_notification;
        });
        connect(options_window, &Options::audioDeviceSet, [this] (const QString& description) {
            auto device_list = ui->playerWidget->getAllAudioDevice();
            for (auto device : device_list) {
                if (!device.description().compare(description)) {
                    ui->playerWidget->changeAudioDevice(device);
                }
            }
        });
        connect(options_window, &Options::themeChanged, [this] (const QString& path) {
            // Switch Theme Config
            if (path.isEmpty()) {
                setStyleSheet("");
                return;
            }
            QFile file(path);
            if (file.open(QIODevice::ReadOnly)) {
                setStyleSheet(file.readAll());
                file.close();
                app_options.theme_path = path;
            } else {
                setStyleSheet("");
            }
        });
    }
    auto audio_list = ui->playerWidget->getAllAudioDevice();
    // 获取当前的输出设备
    options_window->setAudioDeviceList(audio_list);
    options_window->getCurrentAudioDevice(ui->playerWidget->getCurrentAudioDevice());
    options_window->setupAppConfig(app_options);
    options_window->show();
}

void MusicPlayer::playPause() {
    if (ui->playerWidget->isPlayingSong()) {
        ui->playerWidget->pause();
    } else {
        ui->playerWidget->play();
    }
}

void MusicPlayer::getSongInfo(const QString &url) {
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
    auto file_name = QFileDialog::getOpenFileName(this, "导入歌曲到播放列表", QDir::homePath(), "MP3文件(*.mp3);;FLAC文件(*.flac);;OGG文件(*.ogg);;音频文件(*.wav);;所有文件(*.*)");
    if (file_name.isEmpty()) return;
    Playlist::Song temp_song;
    uint idx = playlist.findSongByPath(file_name);
    if (idx != UINT16_MAX) return;
    if (playlist.parseSongFromFileName(file_name, temp_song)) {
        addSongToPlaylist(temp_song);
    } else {
        qCritical().noquote() << "[Error] File" << file_name << "can not parse!";
        QMessageBox::critical(this, "无法导入", QString("文件 %1 不是有效或可解析的音乐文件！").arg(file_name));
    }
    status_list->setText(QString("当前播放列表共 %1 项").arg(model->rowCount()));
}

void MusicPlayer::importSongDirectoryToList() {
    auto dir_name = QFileDialog::getExistingDirectory(this, "导入歌曲目录到列表", QDir::homePath());
    if (dir_name.isEmpty()) return;
//    qDebug() << "Import Directory: " << dir_name;
    if (!import_task) {
        import_task = new ImportSongsTask();
//        qDebug() << "Create Import task!";
        connect(import_task, &ImportSongsTask::finishedWork, [this] (int count) {
            import_task->clearImportedPlaylist();
            status_list->setText(QString("此次共导入了 %1 首歌曲！").arg(count));
        });
        connect(import_task, &ImportSongsTask::addSong, [this] (const Playlist::Song& song) {
            uint idx = playlist.findSongByPath(song.path);
            if (idx == UINT16_MAX) {
                playlist.addSongToList(song);
                addSongToPlaylist(song);
            }
        });
    }
    import_task->setDirectory(dir_name);
    import_task->start();
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
        if (current_playing == model->rowCount() && model->rowCount() > 0) {
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
    aboutApp.setStyleSheet(styleSheet());
    aboutApp.exec();
}

void ImportSongsTask::run() {
    qDebug() << "Import Task is started!";
    QDir dir(directory_name);
    int success_count = 0;
    auto songs_list = dir.entryList(QDir::Files);
    for (auto& song : songs_list) {
        QString path = directory_name + "/" + song;
        qDebug() << "Import:" << song;
        Playlist::Song temp_song;
        if (imported_play_list.parseSongFromFileName(path, temp_song)) {
            success_count++;
            emit addSong(temp_song);
        }
    }
    qDebug() << "Import finished!\n- Successful:" << success_count;
    emit finishedWork(success_count);
}

const Playlist& ImportSongsTask::getImportedPlaylist() {
    return imported_play_list;
}

void ImportSongsTask::clearImportedPlaylist() {
    imported_play_list.clearPlaylist();
}

AddMultiSongsTask::AddMultiSongsTask() {}

AddMultiSongsTask::AddMultiSongsTask(QList<QString> &urls) {
    for (auto& i : urls) {
        path_list.emplace_back(i);
    }
}

void AddMultiSongsTask::setTaskList(const QList<QString> &urls) {
    for (auto& i : urls) {
        path_list.emplace_back(i);
    }
}

void AddMultiSongsTask::clearTaskList() {
    added_playlist.clearPlaylist();
    path_list.clear();
}

void AddMultiSongsTask::run() {
    uint success_count = 0;
    for (auto& song : path_list) {
        qDebug() << "Import:" << song;
        Playlist::Song temp_song;
        if (added_playlist.parseSongFromFileName(song, temp_song)) {
            success_count++;
            emit addSongToList(temp_song);
        }
    }
    qDebug() << "Import finished!\n- Successful:" << success_count;

    emit finishedWork();
}
