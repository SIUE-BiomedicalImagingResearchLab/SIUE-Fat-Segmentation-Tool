#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    int ret = -1;

    try
    {
        QApplication a(argc, argv);
        QCoreApplication::setOrganizationName("Southern Illinois University Edwardsville");
        QCoreApplication::setApplicationName("Visceral Fat Validation");
        QCoreApplication::setApplicationVersion(QT_VERSION_STR);
        MainWindow w;
        w.show();
        ret = a.exec();
    }
    catch (const std::logic_error &e)
    {
        QMessageBox::critical(NULL, "Logic Error", e.what());
    }
    catch (const std::runtime_error &e)
    {
        QMessageBox::critical(NULL, "Runtime Error", e.what());
    }
    catch (const Exception &e)
    {
        QMessageBox::critical(NULL, e.title(), e.message());
    }
    catch (...)
    {
        QMessageBox::critical(NULL, "Unknown Error", "An unknown error occurred.");
    }

    return ret;
}
