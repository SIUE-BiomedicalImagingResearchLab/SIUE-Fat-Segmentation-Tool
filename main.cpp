#include "mainwindow.h"
#include "application.h"

#include <opencv2/opencv.hpp>

int main(int argc, char *argv[])
{
    int ret = -1;

    try
    {
        Application a(argc, argv);
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
        qCritical() << "Logic Error: " << e.what();
    }
    catch (const std::runtime_error &e)
    {
        QMessageBox::critical(NULL, "Runtime Error", e.what());
        qCritical() << "Runtime Error: " << e.what();
    }
    catch (const Exception &e)
    {
        QMessageBox::critical(NULL, e.title(), e.message());
        qCritical() << "Error " << e.title() << " : " << e.message();
    }
    catch (const cv::Exception &e)
    {
        QMessageBox::critical(NULL, "OpenCV Error", e.what());
        qCritical() << "OpenCV Error: " << e.what();
    }
    catch (...)
    {
        QMessageBox::critical(NULL, "Unknown Error", "An unknown error occurred.");
        qCritical() << "An unknown error was thrown";
    }

    return ret;
}
