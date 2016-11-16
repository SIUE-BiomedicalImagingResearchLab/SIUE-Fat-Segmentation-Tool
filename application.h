#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QtGui>

#include <opencv2/opencv.hpp>

#include "exception.hpp"

class Application : public QApplication
{

public:
    Application(int &argc, char **argv) : QApplication(argc, argv) {}
    virtual ~Application() {}

    virtual bool notify(QObject *receiver, QEvent *event)
    {
        try
        {
            return QApplication::notify(receiver, event);
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

        return false;
    }
};

#endif // APPLICATION_H
