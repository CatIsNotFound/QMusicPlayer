

#include "Player.h"
#include "ui_Player.h"


Player::Player(QWidget *parent) :
        QWidget(parent), ui(new Ui::Player) {
    ui->setupUi(this);
    mediaPlayer = new QMediaPlayer();
    audioOutput = new QAudioOutput;
    audioOutput->setDevice(QMediaDevices::defaultAudioOutput());
    mediaPlayer->setAudioOutput(audioOutput);
    delayer = new QTimer(this);
    delayer->setSingleShot(true);
    connect(delayer, &QTimer::timeout, [this] {
        playing = false;
        emit isNotRealPlaying();
    });
    connect(this, &Player::isNotRealPlaying, [this] {
        if (!playing && mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
            playing = true;
            mediaPlayer->stop();
            mediaPlayer->play();
        }
    });
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &Player::positionChanged);
    connect(mediaPlayer, &QMediaPlayer::mediaStatusChanged, [this] (QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia && playLoopMode == PlayLoopMode::ListLoop) {
            emit nextSongRequest();
        } else if (status == QMediaPlayer::EndOfMedia && playLoopMode == PlayLoopMode::NoLoop) {
            stop();
        } else if (status == QMediaPlayer::InvalidMedia) {
            if (playLoopMode == PlayLoopMode::ListLoop) {
                ui->label_playSong->setText("Invalid Media! Next one will play after 3 sec!");
                QTimer::singleShot(3000, [&] { emit nextSongRequest(); });
            } else {
                ui->label_playSong->setText("Invalid Media!");
            }
            emit invalidMedia(mediaPlayer->source().path());
        }
    });
    connect(ui->toolButton_PlayPauseSong, &QToolButton::clicked, this, &Player::playAndPause);
    connect(ui->toolButton_StopSong, &QToolButton::clicked, this, &Player::stop);
    connect(ui->toolButton_Volume, &QToolButton::clicked, this, [&] {
        ui->widget_volume->move(ui->toolButton_Volume->x() - ui->widget_volume->width() / 2,
                                ui->toolButton_Volume->y() - ui->widget_volume->height() - 10);
        ui->widget_volume->setVisible(!ui->widget_volume->isVisible());
    });
    connect(ui->toolButton_Backward, &QToolButton::clicked, this, &Player::backward3Sec);
    connect(ui->toolButton_Forward, &QToolButton::clicked, this, &Player::forward3Sec);
    connect(ui->toolButton_LastSong, &QToolButton::clicked, [&] { emit lastSongRequest(); });
    connect(ui->toolButton_NextSong, &QToolButton::clicked, [&] { emit nextSongRequest(); });
    connect(ui->toolButton_PlayMode, &QToolButton::clicked, [this] {
        if (playLoopMode == PlayLoopMode::NoLoop) {
            setPlayLoopMode(PlayLoopMode::SingleSongLoop);
        } else if (playLoopMode == PlayLoopMode::SingleSongLoop) {
            setPlayLoopMode(PlayLoopMode::ListLoop);
        } else if (playLoopMode == PlayLoopMode::ListLoop) {
            setPlayLoopMode(PlayLoopMode::NoLoop);
        }
    });

    connect(ui->volumeSlider, &QSlider::valueChanged, this, [&](int value){
        if (audioOutput->isMuted()) {
            if (isMuted()) muteVolume();
        }
        audioOutput->setVolume(value / 100.0);
        ui->label_volume->setText(QString::number(ui->volumeSlider->value()));
        if (value > 0) {
            ui->toolButton_Volume->setToolTip(QString("音量：%1%").arg(ui->volumeSlider->value()));
            ui->toolButton_Volume->setIcon(QIcon(":/assets/声音-大_volume-notice.svg"));
        } else {
            if (!isMuted()) muteVolume();
        }
    });
    connect(mediaPlayer, &QMediaPlayer::metaDataChanged, this, &Player::metaDataChanged);
    connect(ui->playPositionSlider, &QSlider::sliderPressed, [&]{ isSliderMoved = true; });
    connect(ui->playPositionSlider, &QSlider::sliderReleased, [&]{ isSliderMoved = false; changePlayPosition(); });
    // 其它初始化
    ui->toolButton_PlayPauseSong->setIcon(QIcon(":/assets/playSong.svg"));
    setEnabledControls(false);
    ui->volumeSlider->setMaximum(100);
    ui->volumeSlider->setValue(audioOutput->volume() * 100);
    ui->toolButton_Volume->setToolTip(QString("音量：%1%").arg(ui->volumeSlider->value()));
    ui->playPositionSlider->setMaximum(100);
    setPlayLoopMode(PlayLoopMode::ListLoop);
}

Player::~Player() {
    delete ui;
}

void Player::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    ui->verticalLayoutWidget->setGeometry(QRect(0, 0, this->width(), this->height()));
    ui->widget_volume->move(ui->toolButton_Volume->x() - ui->widget_volume->width() / 2,
                            ui->toolButton_Volume->y() - ui->widget_volume->height() - 10);
}

void Player::positionChanged(qint64 position) {
    if (!mediaPlayer->isAvailable()) {
        stop();
        ui->label_playSong->setText("Invalid Media!");
        emit invalidMedia(mediaPlayer->source().toString());
    }
    qint64 p = position / 1000;
    int min = p / 60;
    int sec = p % 60;
    qint64 duration = mediaPlayer->metaData().value(QMediaMetaData::Duration).toLongLong();
    qint64 pa = duration / 1000;
    ui->label_playProcess->setText(QString::asprintf("%d:%02d / %lld:%02lld", min, sec, pa / 60, pa % 60));
    if (!isSliderMoved) ui->playPositionSlider->setValue(position / (duration * 1.0) * 100);
}

void Player::setEnabledControls(const bool enabled) {
    ui->toolButton_StopSong->setEnabled(enabled);
    ui->toolButton_PlayPauseSong->setEnabled(enabled);
    ui->toolButton_Backward->setEnabled(enabled);
    ui->toolButton_Forward->setEnabled(enabled);
    ui->toolButton_LastSong->setEnabled(enabled);
    ui->toolButton_NextSong->setEnabled(enabled);
    ui->toolButton_Volume->setEnabled(enabled);
    ui->playPositionSlider->setEnabled(enabled);
    ui->widget_volume->setVisible(false);
    emit playControlChanged(enabled);
}

void Player::setPlayerControls(const bool enabled) {
    ui->toolButton_StopSong->setEnabled(enabled);
    ui->toolButton_Backward->setEnabled(enabled);
    ui->toolButton_Forward->setEnabled(enabled);
    ui->playPositionSlider->setEnabled(enabled);
    emit playControlChanged(enabled);
}

const bool Player::isEnabledControl() const { return ui->playPositionSlider->isEnabled(); }

void Player::playAndPause() {
    if (playing) {
        pause();
    } else {
        play();
    }
}

void Player::play() {
    if (mediaPlayer->mediaStatus() == QMediaPlayer::NoMedia) {
        ui->label_playSong->setText("No Media Found!");
    } else {
        mediaPlayer->play();
        playing = true;
        ui->toolButton_PlayPauseSong->setIcon(QIcon(":/assets/pauseSong.svg"));
        setPlayerControls(true);
    }
}

void Player::pause() {
    mediaPlayer->pause();
    playing = false;
    ui->toolButton_PlayPauseSong->setIcon(QIcon(":/assets/playSong.svg"));
}

void Player::stop() {
    mediaPlayer->stop();
    playing = false;
    ui->toolButton_PlayPauseSong->setIcon(QIcon(":/assets/playSong.svg"));
    setPlayerControls(false);
}

void Player::playSongImmediately(const QString &url) {
    stop();
    // 对于打开完全一样的路径，直接忽略更改
    QUrl newUrl = QUrl::fromLocalFile(url);
    if (mediaPlayer->source() == newUrl) {
        play();
        setEnabledControls(true);
        return;
    }
    mediaPlayer->setSource(newUrl);
    play();
    if (!QSysInfo::kernelType().compare("linux")) {
        delayer->start(500);
    }
    setEnabledControls(true);
    emit songsChanged(url);
}

const bool Player::isPlayingSong() const { return playing; }

const int Player::volume() const { return audioOutput->volume() * 100; }

const bool Player::isMuted() const { return audioOutput->isMuted(); }

void Player::dropOutSong() {
    stop();
    mediaPlayer->setSource(QUrl());
    setEnabledControls(false);
    ui->label_playSong->setText("No Music is Playing!");
    emit resetPlayer();
}

void Player::setPlayLoopMode(const PlayLoopMode &loopMode) {
    playLoopMode = loopMode;
    switch (loopMode) {
        case PlayLoopMode::NoLoop:
            ui->toolButton_PlayMode->setIcon(QIcon(":/assets/播放一次_play-once.svg"));
            ui->toolButton_PlayMode->setToolTip("播放一次");
            mediaPlayer->setLoops(1);
            break;
        case PlayLoopMode::ListLoop:
            ui->toolButton_PlayMode->setIcon(QIcon(":/assets/循环播放_play-cycle.svg"));
            ui->toolButton_PlayMode->setToolTip("列表循环");
            mediaPlayer->setLoops(1);
            break;
        case PlayLoopMode::SingleSongLoop:
            ui->toolButton_PlayMode->setIcon(QIcon(":/assets/循环一次_loop-once.svg"));
            ui->toolButton_PlayMode->setToolTip("单曲循环");
            mediaPlayer->setLoops(-1);
            break;
    }
}

const PlayLoopMode& Player::getPlayLoopMode() const { return playLoopMode; }

void Player::changeVolume(const int volume) {
    audioOutput->setMuted(false);
    if (volume > 100) {
        audioOutput->setVolume(1);
        ui->volumeSlider->setValue(100);
    }
    else if (volume < 0) {
        audioOutput->setVolume(0);
        ui->volumeSlider->setValue(0);
    } else {
        audioOutput->setVolume(volume / 100.0);
        ui->volumeSlider->setValue(volume);
    }
    if (volume <= 0) {
        ui->toolButton_Volume->setIcon(QIcon(":/assets/静音_volume-mute.svg"));
    } else {
        ui->toolButton_Volume->setIcon(QIcon(":/assets/声音-大_volume-notice.svg"));
    }
    emit volumeChanged(volume);
}

void Player::muteVolume() {
    audioOutput->setMuted(!audioOutput->isMuted());
    emit muteVolumeChanged(isMuted());
    if (isMuted()) {
        ui->toolButton_Volume->setIcon(QIcon(":/assets/静音_volume-mute.svg"));
        ui->toolButton_Volume->setToolTip("已静音");
    } else {
        if (ui->volumeSlider->value() != 0) ui->toolButton_Volume->setIcon(QIcon(":/assets/声音-大_volume-notice.svg"));
        ui->toolButton_Volume->setToolTip(QString("音量：%1%").arg(ui->volumeSlider->value()));
    }
}

void Player::changeAudioDevice(const QAudioDevice &audioDevice) {
    audioOutput->setDevice(audioDevice);
    emit deviceChanged();
}

QList<QAudioDevice> Player::getAllAudioDevice() {
    return QMediaDevices::audioOutputs();
}

void Player::metaDataChanged() {
    auto metadata = mediaPlayer->metaData();
    if (metadata.stringValue(QMediaMetaData::Title).isEmpty()) {
        auto name = mediaPlayer->source().toString();
        ui->label_playSong->setText(name.right(name.length() - name.lastIndexOf('/') - 1));
        return;
    }
    if (metadata.stringValue(QMediaMetaData::Author).isEmpty()) {
        ui->label_playSong->setText(
                QString("%1")
                        .arg(metadata.value(QMediaMetaData::Title).toString())
                        );
        return;
    }
    ui->label_playSong->setText(
            QString("%1 - %2")
                    .arg(metadata.stringValue(QMediaMetaData::Title))
                    .arg(metadata.stringValue(QMediaMetaData::Author))
    );
    auto temp_data = metadata.value(QMediaMetaData::CoverArtImage);
    if (temp_data.canConvert<QImage>()) {
        QImage cover_image = temp_data.value<QImage>();
        if (!cover_image.isNull()) {
            QPixmap m_image = QPixmap::fromImage(cover_image)
                        .scaled(ui->label_playImage->width(), ui->label_playImage->height(),
                                Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->label_playImage->setPixmap(m_image);
        } else {
            QPixmap m_image = QPixmap::fromImage(QImage(":/assets/Default.jpg"))
                    .scaled(ui->label_playImage->width(), ui->label_playImage->height(),
                            Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->label_playImage->setPixmap(m_image);
        }
    }
}

void Player::changePlayPosition() {
    int v = ui->playPositionSlider->value();
    qint64 duration = mediaPlayer->metaData().value(QMediaMetaData::Duration).toLongLong();
    mediaPlayer->setPosition(duration * (v / 100.0));
}

void Player::backward3Sec() {
    int64_t pos = mediaPlayer->position();
    if (pos - 3000 <= 0) {
        mediaPlayer->setPosition(0);
    } else {
        mediaPlayer->setPosition(pos - 3000);
    }
}

void Player::forward3Sec() {
    int64_t pos = mediaPlayer->position();
    mediaPlayer->setPosition(pos + 3000);
}
