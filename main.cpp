#include "mainwindow.h"
#include "application.h"

#include <opencv2/opencv.hpp>

MainWindow *w = NULL;
Application *app = NULL;

void messageLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Open the stdout as a text stream and write the formatted log message
    QTextStream out(stdout, QIODevice::WriteOnly);
    out << qFormatLogMessage(type, context, msg) << endl;
    out.flush(); // Flush so that the changes are seen immediately

    switch (type)
    {
        case QtDebugMsg:
            break;

        case QtInfoMsg:
            break;

        // For warning, critical, and fatal errors, show a message box to the user
        // Note: Displaying a message box before the window is initialized causes it to crash
        case QtWarningMsg:
            if (w)
                QMessageBox::warning(w, QObject::tr("%1:%2").arg(context.function).arg(context.line), msg, QMessageBox::Ok);
            break;

        case QtCriticalMsg:
            if (w)
                QMessageBox::critical(w, QObject::tr("%1:%2").arg(context.function).arg(context.line), msg, QMessageBox::Ok);
            break;

        case QtFatalMsg:
            if (w)
                QMessageBox::critical(w, QObject::tr("%1:%2").arg(context.function).arg(context.line), msg, QMessageBox::Ok);
            abort();
    }
}

int main(int argc, char *argv[])
{
    int ret = -1;

    try
    {
        // Note: This will send all output from stdout to the error.log file.
        // This should be uncommented when deploying the application so that the user will be able to see errors.
        // However, if compiling within Qt Creator, it is usually better to comment this out so the output will go to the
        // Qt Creator box instead.
        if (!freopen("./error.log", "w", stdout))
            qWarning() << "Unable to redirect stdout to ./error.log. Errors will not be logged in a text file";

        qInstallMessageHandler(messageLogger);
        qSetMessagePattern("[%{time MM/dd/yyyy h:mm:ss}] [%{if-debug}Debug%{endif}%{if-info}Info%{endif}%{if-warning}Warn%{endif}%{if-critical}Crit%{endif}%{if-fatal}Fatal%{endif}] %{function}:%{line} - %{message}");
        app = new Application(argc, argv);
        QCoreApplication::setOrganizationName("Southern Illinois University Edwardsville");
        QCoreApplication::setApplicationName("Visceral Fat Validation");
        QCoreApplication::setApplicationVersion(APP_VERSION);
        w = new MainWindow();

        w->show();
        ret = app->exec();
    }
    catch (const std::logic_error &e)
    {
        qCritical() << "Logic Error: " << e.what();
    }
    catch (const std::runtime_error &e)
    {
        qCritical() << "Runtime Error: " << e.what();
    }
    catch (const Exception &e)
    {
        qCritical() << "Error " << e.title() << " : " << e.message();
    }
    catch (const cv::Exception &e)
    {
        qCritical() << "OpenCV Error: " << e.what();
    }
    catch (...)
    {
        qCritical() << "An unknown error was thrown";
    }

    // Only delete the pointers if they were allocated in the first place,
    // sometimes errors happen before they were allocated
    if (w)
        delete w;

    if (app)
        delete app;

    return ret;
}
