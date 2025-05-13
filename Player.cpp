

#include "Player.h"
#include "ui_Player.h"


Player::Player(QWidget *parent) :
        QWidget(parent), ui(new Ui::Player) {
    ui->setupUi(this);
    mediaPlayer = new QMediaPlayer();
    audioOutput = new QAudioOutput;
    audioOutput->setDevice(QMediaDevices::defaultAudioOutput());
    mediaPlayer->setAudioOutput(audioOutput);
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &Player::positionChanged);
    connect(ui->toolButton_PlayPauseSong, &QToolButton::clicked, this, &Player::playAndPause);
    connect(ui->toolButton_StopSong, &QToolButton::clicked, this, &Player::stop);
    connect(ui->toolButton_Volume, &QToolButton::clicked, this, [&] {
        ui->widget_volume->setVisible(!ui->widget_volume->isVisible());
    });
    connect(ui->toolButton_Backward, &QToolButton::clicked, this, &Player::backward3Sec);
    connect(ui->toolButton_Forward, &QToolButton::clicked, this, &Player::forward3Sec);
    connect(ui->toolButton_LastSong, &QToolButton::clicked, [&] { emit lastSongRequest(); });
    connect(ui->toolButton_NextSong, &QToolButton::clicked, [&] { emit nextSongRequest(); });

    connect(ui->volumeSlider, &QSlider::valueChanged, this, [&](int value){
        audioOutput->setVolume(value / 100.0);
        ui->label_volume->setText(QString::number(ui->volumeSlider->value()));
    });
    connect(mediaPlayer, &QMediaPlayer::metaDataChanged, this, &Player::metaDataChanged);
    connect(ui->playPositionSlider, &QSlider::sliderPressed, [&]{ isSliderMoved = true; });
    connect(ui->playPositionSlider, &QSlider::sliderReleased, [&]{ isSliderMoved = false; changePlayPosition(); });
    // 其它初始化
    ui->toolButton_PlayPauseSong->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
    setEnabledControls(false);
    ui->volumeSlider->setMaximum(100);
    ui->volumeSlider->setValue(audioOutput->volume() * 100);
    ui->playPositionSlider->setMaximum(100);
}

Player::~Player() {
    delete ui;
}

void Player::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    ui->verticalLayoutWidget->setGeometry(QRect(0, 0, this->width(), this->height()));
}

void Player::positionChanged(qint64 position) {
    if (mediaPlayer->error() != QMediaPlayer::NoError) {
        stop();
        ui->label_playSong->setText("Invalid Media!");
        emit invalidMedia(mediaPlayer->source().toString());
    }
    qint64 p = position / 1000;
    int min = p / 60;
    int sec = p % 60;
    qint64 duration = mediaPlayer->metaData().value(QMediaMetaData::Duration).toULongLong();
    qint64 pa = duration / 1000;
    ui->label_playProcess->setText(QString::asprintf("%d:%02d / %d:%02d", min, sec, pa / 60, pa % 60));
    if (!isSliderMoved) ui->playPositionSlider->setValue(position / (duration * 1.0) * 100);
    // 当歌曲即将结束
    if (position >= duration) {
        stop();     // 默认结束
    }
}

void Player::setEnabledControls(const bool enabled) {
    ui->toolButton_StopSong->setEnabled(enabled);
    ui->toolButton_PlayPauseSong->setEnabled(enabled);
    ui->toolButton_Backward->setEnabled(enabled);
    ui->toolButton_Forward->setEnabled(enabled);
    ui->toolButton_LastSong->setEnabled(enabled);
    ui->toolButton_NextSong->setEnabled(enabled);
    ui->toolButton_Volume->setEnabled(enabled);
    ui->widget_volume->setVisible(false);
}

void Player::playAndPause() {
    if (mediaPlayer->isPlaying()) {
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
        ui->toolButton_PlayPauseSong->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause));
    }
}

void Player::pause() {
    mediaPlayer->pause();
    ui->toolButton_PlayPauseSong->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
}

void Player::stop() {
    mediaPlayer->stop();
    ui->toolButton_PlayPauseSong->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
}

void Player::playSongImmediately(const QString &url) {
    stop();
    mediaPlayer->setSource(QUrl::fromLocalFile(url));
    // 检查是否能播放
    play();
    setEnabledControls(true);

    emit songsChanged(url);
}

const bool Player::isPlayingSong() const { return mediaPlayer->isPlaying(); }

void Player::changeVolume(const quint8 volume) {
    audioOutput->setVolume(volume);
    emit volumeChanged(volume);
}

void Player::changeAudioDevice(const QAudioDevice &audioDevice) {
    audioOutput->setDevice(audioDevice);
}

QList<QAudioDevice> Player::getAllAudioDevice() {
    return QMediaDevices::audioOutputs();
}

void Player::metaDataChanged() {
    auto metadata = mediaPlayer->metaData();
    ui->label_playSong->setText(metadata.value(QMediaMetaData::Title).toString());
    qDebug() << "Media Cover: " << metadata.value(QMediaMetaData::CoverArtImage).toString();
}

void Player::changePlayPosition() {
    int v = ui->playPositionSlider->value();
    qint64 duration = mediaPlayer->metaData().value(QMediaMetaData::Duration).toULongLong();
    mediaPlayer->setPosition(duration * (v / 100.0));
}

void Player::backward3Sec() {
    uint64_t pos = mediaPlayer->position();
    if (pos - 3000 <= 0) {
        mediaPlayer->setPosition(0);
    } else {
        mediaPlayer->setPosition(pos - 3000);
    }
}

void Player::forward3Sec() {
    uint64_t pos = mediaPlayer->position();
    mediaPlayer->setPosition(pos + 3000);
}
