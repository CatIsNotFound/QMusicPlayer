
#include "AboutApp.h"
#include "ui_AboutApp.h"


AboutApp::AboutApp(QWidget *parent) :
        QDialog(parent), ui(new Ui::AboutApp) {
    ui->setupUi(this);
    ui->label_appname->setText(QApplication::applicationName());
    ui->label_appversion->setText(QApplication::applicationVersion());
}

AboutApp::~AboutApp() {
    delete ui;
}
