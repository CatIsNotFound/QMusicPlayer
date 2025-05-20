
#ifndef QMUSICPLAYER_OPTIONS_H
#define QMUSICPLAYER_OPTIONS_H

#include <QDialog>
#include <QAudioDevice>
#include <QList>
#include <QFileDialog>
#include "AppConfig.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Options; }
QT_END_NAMESPACE

class Options : public QDialog {
Q_OBJECT

public:
    explicit Options(QWidget *parent = nullptr);

    ~Options() override;

    void setAudioDeviceList(const QList<QAudioDevice>& list);
    void getCurrentAudioDevice(const QAudioDevice& audio_device);
    void setupAppConfig(const AppOptions& app_options);
    void saveAppConfig();
    bool isExistThemeFile(const QString& file_path);

signals:
    void audioDeviceSet(const QString& desription);
    void ConfigSaved(AppOptions& app_options);
    void themeChanged(const QString& path);

private:
    Ui::Options *ui;
    QList<QAudioDevice> audio_devices;

};


#endif //QMUSICPLAYER_OPTIONS_H
