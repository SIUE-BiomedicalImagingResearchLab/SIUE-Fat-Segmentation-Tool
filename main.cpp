#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Southern Illinois University Edwardsville");
    QCoreApplication::setApplicationName("Visceral Fat Validation");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    MainWindow w;
    w.show();

    return a.exec();
}
