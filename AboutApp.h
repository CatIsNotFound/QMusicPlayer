
#ifndef QMUSICPLAYER_ABOUTAPP_H
#define QMUSICPLAYER_ABOUTAPP_H

#include <QDialog>
#include <QDesktopServices>

QT_BEGIN_NAMESPACE
namespace Ui { class AboutApp; }
QT_END_NAMESPACE

class AboutApp : public QDialog {
Q_OBJECT

public:
    explicit AboutApp(QWidget *parent = nullptr);

    ~AboutApp() override;

private:
    Ui::AboutApp *ui;
};


#endif //QMUSICPLAYER_ABOUTAPP_H
