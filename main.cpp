#include <QApplication>
#include <QPushButton>
#include "MusicPlayer.h"
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setApplicationName("QMusicPlayer");
    a.setApplicationVersion("v0.1.1-beta");
    a.setWindowIcon(QIcon(":/assets/appicon.ico"));
    MusicPlayer player;
    player.show();
    return QApplication::exec();
}
