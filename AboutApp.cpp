
#include "AboutApp.h"
#include "ui_AboutApp.h"


AboutApp::AboutApp(QWidget *parent) :
        QDialog(parent), ui(new Ui::AboutApp) {
    ui->setupUi(this);
    ui->label_appname->setText(QApplication::applicationName());
    ui->label_appversion->setText(QApplication::applicationVersion());
    connect(ui->pushButton_Release, &QPushButton::clicked, this, []{
        QDesktopServices::openUrl(QUrl("https://github.com/CatIsNotFound/QMusicPlayer/releases/latest"));
    });
    setMinimumSize(width(), height());
    setMaximumSize(width(), height());
}

AboutApp::~AboutApp() {
    delete ui;
}
