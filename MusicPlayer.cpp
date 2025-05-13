
#include "MusicPlayer.h"
#include "ui_MusicPlayer.h"


MusicPlayer::MusicPlayer(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MusicPlayer) {
    ui->setupUi(this);
    // ui->playerWidget = new Player(ui->splitter);
    trayMenu = new QMenu(this);
    action_ShowUI = new QAction("显示主界面(&S)");
    action_Quit = new QAction("退出(&Q)");
    trayMenu->addAction(action_ShowUI);
    trayMenu->addAction(action_Quit);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QApplication::windowIcon());
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
    trayIcon->setToolTip(QApplication::applicationName());

    setupConnection();
    setupList();
    setWindowTitle(QApplication::applicationName());
}

MusicPlayer::~MusicPlayer() {
    delete ui;
}

void MusicPlayer::setupConnection() {
    // 系统后台
    connect(action_ShowUI, &QAction::triggered, this, &MusicPlayer::show);
    connect(action_Quit, &QAction::triggered, this, &QApplication::quit);
    connect(trayIcon, &QSystemTrayIcon::activated, this, [&] (QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) this->show();
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

    // 帮助

    connect(ui->action_AboutQt, &QAction::triggered, this, &QApplication::aboutQt);

    // 播放器
    connect(ui->playerWidget, &Player::invalidMedia, this, [this](const QString& url) {
        QMessageBox::critical(this, "无法打开", QString("无法打开并播放文件 %1，可能此文件不是有效的可播放音乐文件！").arg(url));
    });
}

void MusicPlayer::setupList() {
    model = new QStandardItemModel;
    table = new QTableView();
    QStringList headers;
    headers << "文件路径" << "歌曲名" << "歌手" << "专辑";
    model->setHorizontalHeaderLabels(headers);
    table->setModel(model);
    ui->tabWidget->clear();
    ui->tabWidget->addTab(table, "试听列表");
}

void MusicPlayer::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    ui->splitter->setGeometry(QRect(0, 0, ui->centralwidget->width(), ui->centralwidget->height()));
}

void MusicPlayer::closeEvent(QCloseEvent *event) {
    event->ignore();
    trayIcon->showMessage(QApplication::applicationName(), "程序已放到后台运行！");
    hide();
}

void MusicPlayer::openFileAndPlayFile() {
    QFileDialog fileDialog;
    QString fileName = fileDialog.getOpenFileName(this, "打开...", QDir::homePath(), "(*.mp3) MP3 文件;; (*.wav) 音频文件;; (*.*) 所有文件");
    if (fileName.isEmpty()) return;
    qDebug() << "Open:" << fileName;
    ui->playerWidget->playSongImmediately(fileName);
}

void MusicPlayer::playPause() {
    if (ui->playerWidget->isPlayingSong()) {
        ui->playerWidget->pause();
    } else {
        ui->playerWidget->play();
    }
}

