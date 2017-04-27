#include "mainwindow.h"
#include "application.h"
#include "stacktrace.h"

#include <opencv2/opencv.hpp>

MainWindow *w = NULL;
Application *app = NULL;
FILE *logFh = NULL;

void messageLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString formattedMsg = qFormatLogMessage(type, context, msg);

    // Open the log file and write the formatted log message
    QTextStream out(logFh, QIODevice::WriteOnly);
    out << formattedMsg << endl;
    out.flush(); // Flush so that the changes are seen immediately

    switch (type)
    {
        case QtDebugMsg:
            std::cout << formattedMsg.toStdString() << std::endl;
            break;

        case QtInfoMsg:
            std::cout << formattedMsg.toStdString() << std::endl;
            break;

        // For warning, critical, and fatal errors, show a message box to the user
        // Note: Displaying a message box before the window is initialized causes it to crash
        case QtWarningMsg:
            std::cerr << formattedMsg.toStdString() << std::endl;
            if (w)
                QMessageBox::warning(w, QObject::tr("%1:%2").arg(context.function).arg(context.line), msg, QMessageBox::Ok);
            break;

        case QtCriticalMsg:
            std::cerr << formattedMsg.toStdString() << std::endl;
            if (w)
                QMessageBox::critical(w, QObject::tr("%1:%2").arg(context.function).arg(context.line), msg, QMessageBox::Ok);
            break;

        case QtFatalMsg:
            std::cerr << formattedMsg.toStdString() << std::endl;
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
        // Open the log file that will record all information sent to the console
        if (!(logFh = fopen("./error.log", "a")))
            qWarning() << "Unable to open log file ./error.log. Errors will not be logged in a text file";

        qInstallMessageHandler(messageLogger);
        qSetMessagePattern("[%{time MM/dd/yyyy h:mm:ss}] [%{if-debug}Debug%{endif}%{if-info}Info%{endif}%{if-warning}Warn%{endif}%{if-critical}Crit%{endif}%{if-fatal}Fatal%{endif}] %{function}:%{line} - %{message}");

        // Set globalProgramName for stack trace retriever. Set signal handler to print stack trace on exception
        globalProgramName = argv[0];
        setSignalHandler();

        app = new Application(argc, argv);
        QCoreApplication::setOrganizationName("Southern Illinois University Edwardsville");
        QCoreApplication::setApplicationName("Visceral Fat Validation");
        QCoreApplication::setApplicationVersion(APP_VERSION);

        QSurfaceFormat format;
#ifdef Q_OS_MACOS
        // Mac OS X only allows core profiles to be used
        // QPainter is used in this application which is based off OpenGL 2.0 but the actual OpenGL used in the shaders
        // for drawing slices is OpenGL 3.3.
        // Luckily, Qt 5.9 provides support where it brings QPainter up to date in OpenGL (I think)
        // To build on Mac, YOU MUST USE AT LEAST Qt 5.9!
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setVersion(3, 3);
#else // Q_OS_MACOS
        format.setProfile(QSurfaceFormat::CompatibilityProfile);
#endif // Q_OS_MACOS
        QSurfaceFormat::setDefaultFormat(format);

        w = new MainWindow();
        w->show();

        ret = app->exec();
    }
    catch (const std::logic_error &e)
    {
        qCritical().nospace().noquote() << "Logic Error: " << e.what() << "\n" << printStackTrace();
    }
    catch (const std::runtime_error &e)
    {
        qCritical().nospace().noquote() << "Runtime Error: " << e.what() << "\n" << printStackTrace();
    }
    catch (const Exception &e)
    {
        qCritical().nospace().noquote() << "Error " << e.title() << " : " << e.message() << "\n" << printStackTrace();
    }
    catch (const cv::Exception &e)
    {
        qCritical().nospace().noquote() << "OpenCV Error: " << e.what() << "\n" << printStackTrace();
    }
    catch (...)
    {
        qCritical().nospace().noquote() << "An unknown error was thrown\n" << printStackTrace();
    }

    // Only delete the pointers if they were allocated in the first place,
    // sometimes errors happen before they were allocated
    if (w)
        delete w;

    if (app)
        delete app;

    if (logFh)
        fclose(logFh);

    return ret;
}
