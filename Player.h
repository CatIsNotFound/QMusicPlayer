
#ifndef MUSICPLAYER_PLAYER_H
#define MUSICPLAYER_PLAYER_H

#include <QWidget>
#include <QList>
#include <QMediaPlayer>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioOutput>
#include <QMediaMetaData>
#include <QPointer>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class Player; }
QT_END_NAMESPACE

// 播放循环模式
enum class PlayLoopMode {
    NoLoop,             // 无循环
    ListLoop,           // 列表循环
    SingleSongLoop      // 单曲循环
};

class Player : public QWidget {
Q_OBJECT

public:
    explicit Player(QWidget *parent = nullptr);

    ~Player() override;

    void setEnabledControls(const bool enabled);
    void setPlayerControls(const bool enabled);
    const bool isEnabledControl() const;
    const bool isPlayingSong() const;
    const int volume() const;
    const bool isMuted() const;
    void dropOutSong();
    void setPlayLoopMode(const PlayLoopMode& loopMode);
    const PlayLoopMode& getPlayLoopMode() const;
    QList<QAudioDevice> getAllAudioDevice();
    const QAudioDevice& getCurrentAudioDevice() const;
    void setAutoChangeOutputSwitch(const bool &b);
    void setAutoPausedSwitch(const bool &b);


signals:
    void songsChanged(const QString& url);
    void playControlChanged(const bool& enabled);
    void playPositionChanged(const qint64 position);
    void volumeChanged(const quint8 volume);
    void muteVolumeChanged(const bool& enabled);
    void deviceChanged(const QAudioDevice& device);
    void invalidMedia(const QString& url);
    void nextSongRequest();
    void lastSongRequest();
    void resetPlayer();
    void isNotRealPlaying();

public slots:
    void playAndPause();
    void play();
    void pause();
    void stop();
    void playSongImmediately(const QString& url);
    void setSong(const QString& url);
    void changeVolume(const int volume);
    void muteVolume();
    void changeAudioDevice(const QAudioDevice& audioDevice);
    void metaDataChanged();
    void changePlayPosition();
    void backward3Sec();
    void forward3Sec();
    void sendNextSongRequest();
    void sendLastSongRequest();


protected:
    void resizeEvent(QResizeEvent *event) override;
    void positionChanged(qint64 position);


private:
    Ui::Player *ui;
    QPointer<QMediaDevices> mediaDevices;
    QPointer<QMediaPlayer> mediaPlayer;
    QPointer<QAudioOutput> audioOutput;
    QPointer<QTimer> delayer;
    bool isSliderMoved{false};
    PlayLoopMode playLoopMode{PlayLoopMode::NoLoop};
    bool playing{false};
    bool audioOutputSwitch{false};
    bool audioPausedSwitch{false};
};


#endif //MUSICPLAYER_PLAYER_H
