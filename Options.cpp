

#include "Options.h"
#include "ui_Options.h"


Options::Options(QWidget *parent) :
        QDialog(parent), ui(new Ui::Options) {
    ui->setupUi(this);
    setMinimumSize(width(), height());
    setMaximumSize(width(), height());
    ui->comboBox_themeConfig->addItem("系统配置");
    QString c_dir = QDir::currentPath();
    ui->comboBox_themeConfig->addItem("CoolLight", ":/themes/CoolLight.qss");
    ui->comboBox_themeConfig->addItem("CoolDark", ":/themes/CoolDark.qss");
    connect(ui->pushButton_ApplySettings, &QPushButton::clicked, this, &Options::saveAppConfig);
    connect(ui->pushButton_browser, &QPushButton::clicked, [this] {
        QString file_path = QFileDialog::getOpenFileName(this, "打开主题文件...", QDir::currentPath(), "主题文件(*.qss);;所有文件(*.*)");
        if (!file_path.isEmpty()) {
            QString real_file_name = QDir(file_path).dirName();
            if (!isExistThemeFile(file_path)) {
                ui->comboBox_themeConfig->addItem(real_file_name.left(real_file_name.indexOf('.')));
                int idx = ui->comboBox_themeConfig->count() - 1;
                ui->comboBox_themeConfig->setItemData(idx, file_path);
                ui->comboBox_themeConfig->setCurrentIndex(idx);
            }
        }
    });
}

Options::~Options() {
    delete ui;
}

void Options::setAudioDeviceList(const QList<QAudioDevice> &list) {
    audio_devices.clear();
    ui->comboBox_DefaultAudioDevice->clear();
    // ui->comboBox_DefaultAudioDevice->addItem(QIcon(":/assets/音响_speaker.svg"), "默认输出设备");
    for (auto& device : list) {
        audio_devices.emplace_back(device);
        if (device.description().contains("耳机") || device.description().contains("Headphone", Qt::CaseInsensitive)) {
            ui->comboBox_DefaultAudioDevice->addItem(QIcon(":/assets/耳机_headset-two.svg"), device.description());
        } else {
            ui->comboBox_DefaultAudioDevice->addItem(QIcon(":/assets/音响_sound.svg"), device.description());
        }
    }
}

void Options::getCurrentAudioDevice(const QAudioDevice &audio_device) {
    for (int i = 0; i < audio_devices.size(); ++i) {
        if (!audio_device.id().compare(audio_devices[i].id())) {
            ui->comboBox_DefaultAudioDevice->setCurrentIndex(i);
            break;
        }
    }
}

void Options::setupAppConfig(const AppOptions &app_options) {
    ui->checkBox_PlayMusic->setChecked(app_options.auto_play_music);
    ui->checkBox_autoChangeAudioDevice->setChecked(app_options.auto_change_audio_output);
    ui->checkBox_savePlayingID->setChecked(app_options.remember_playlist_id);
    ui->checkBox_pausedSong->setChecked(app_options.auto_pause_song_when_audio_output_changed);

    ui->checkBox_notify1->setChecked(app_options.close_window_notification);
    ui->checkBox_notify2->setChecked(app_options.start_up_notification);
    ui->checkBox_notify3->setChecked(app_options.change_song_notification);

    if (!isExistThemeFile(app_options.theme_path)) {
        if (!app_options.theme_path.isEmpty()) {
            QString real_file_name = QDir(app_options.theme_path).dirName();

            ui->comboBox_themeConfig->addItem(real_file_name.left(real_file_name.indexOf('.')));
            int idx = ui->comboBox_themeConfig->count() - 1;
            ui->comboBox_themeConfig->setItemData(idx, app_options.theme_path);
        }
    } else {
        for (int i = 0; i < ui->comboBox_themeConfig->count(); ++i) {
            // qDebug() << ui->comboBox_themeConfig->itemData(i).toString() << " = " << app_options.theme_path;
            if (!ui->comboBox_themeConfig->itemData(i).toString().compare(app_options.theme_path)) {
                ui->comboBox_themeConfig->setCurrentIndex(i);
                break;
            }
        }
    }
}

void Options::saveAppConfig() {
    AppOptions app_options;
    app_options.auto_play_music = ui->checkBox_PlayMusic->isChecked();
    app_options.auto_change_audio_output = ui->checkBox_autoChangeAudioDevice->isChecked();
    app_options.remember_playlist_id = ui->checkBox_savePlayingID->isChecked();
    app_options.auto_pause_song_when_audio_output_changed = ui->checkBox_pausedSong->isChecked();

    app_options.close_window_notification = ui->checkBox_notify1->isChecked();
    app_options.start_up_notification = ui->checkBox_notify2->isChecked();
    app_options.change_song_notification = ui->checkBox_notify3->isChecked();

    if (ui->comboBox_themeConfig->currentIndex() > 0) {
        int idx = ui->comboBox_themeConfig->currentIndex();
        app_options.theme_path = ui->comboBox_themeConfig->itemData(idx).toString();
        emit themeChanged(app_options.theme_path);
    } else {
        emit themeChanged("");
    }
    emit audioDeviceSet(ui->comboBox_DefaultAudioDevice->currentText());
    emit ConfigSaved(app_options);
    close();
}

bool Options::isExistThemeFile(const QString& file_path) {
    for (uint i = 0; i < ui->comboBox_themeConfig->count(); ++i) {
        if (!file_path.compare(ui->comboBox_themeConfig->itemData(i).toString())) {
            return true;
        }
    }
    return false;
}



