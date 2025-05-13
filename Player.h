
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

QT_BEGIN_NAMESPACE
namespace Ui { class Player; }
QT_END_NAMESPACE

class Player : public QWidget {
Q_OBJECT

public:
    explicit Player(QWidget *parent = nullptr);

    ~Player() override;

    void setEnabledControls(const bool enabled);
    const bool isPlayingSong() const;

signals:
    void songsChanged(const QString& url);
    void playPositionChanged(const quint64 position);
    void volumeChanged(const quint8 volume);
    void deviceChanged();
    void invalidMedia(const QString& url);
    void nextSongRequest();
    void lastSongRequest();

public slots:
    void playAndPause();
    void play();
    void pause();
    void stop();
    void playSongImmediately(const QString& url);
    void changeVolume(const quint8 volume);
    void changeAudioDevice(const QAudioDevice& audioDevice);
    void metaDataChanged();
    void changePlayPosition();
    void backward3Sec();
    void forward3Sec();


protected:
    void resizeEvent(QResizeEvent *event) override;
    void positionChanged(qint64 position);
    QList<QAudioDevice> getAllAudioDevice();

private:
    Ui::Player *ui;
    QPointer<QMediaPlayer> mediaPlayer;
    QPointer<QAudioOutput> audioOutput;
    bool isSliderMoved{false};
};


#endif //MUSICPLAYER_PLAYER_H
