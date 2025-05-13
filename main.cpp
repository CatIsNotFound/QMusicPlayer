#include <QApplication>
#include <QPushButton>
#include "MusicPlayer.h"
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setApplicationName("Qt 音乐播放器");
    a.setWindowIcon(QIcon(":/assets/appicon.ico"));
    MusicPlayer player;
    player.show();
    return QApplication::exec();
}
