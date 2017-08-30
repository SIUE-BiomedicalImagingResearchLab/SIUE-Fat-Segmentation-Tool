#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    fatImage(new NIFTImage()), waterImage(new NIFTImage()), subConfig(new SubjectConfig()), tracingData(new TracingData()),
    imageZip(NULL), tracingResultsZip(NULL)
{  
    this->fatImage->setSubjectConfig(subConfig);
    this->waterImage->setSubjectConfig(subConfig);

    // Read window settings(size of window from last time application was used) upon initialization
    readSettings();

    this->ui->setupUi(this);

    // Setup the initial view
    this->switchView(windowViewType);

    // If updates have not been checked within the last day, then check for updates
    if (QDateTime::currentDateTime() >= lastUpdateCheck.addDays(1))
        this->checkForUpdates();

    // Apply keyboard shortcuts to the menu items. The benefit of using a predefined
    // key sequence is that it has the list of valid shortcuts for each platform and
    // is applied appropiately. This cannot be done through the Qt GUI Designer
    this->ui->actionOpen->setShortcut(util::getStandardSequence(QKeySequence::Open, QKeySequence("Ctrl+O")));
    this->ui->actionSave->setShortcut(util::getStandardSequence(QKeySequence::Save, QKeySequence("Ctrl+S")));
    this->ui->actionSaveAs->setShortcut(util::getStandardSequence(QKeySequence::SaveAs, QKeySequence("Ctrl+Shift+S")));
    this->ui->actionImportTracingData->setShortcut(QKeySequence("Ctrl+I"));
    this->ui->actionExit->setShortcut(util::getStandardSequence(QKeySequence::Close, QKeySequence("Alt+F4")));
    this->ui->actionUndo->setShortcut(util::getStandardSequence(QKeySequence::Undo, QKeySequence("Ctrl+Z")));

    this->ui->actionRedo->setShortcut(util::getStandardSequence(QKeySequence::Redo, QKeySequence("Ctrl+Shift+Z")));
}

void MainWindow::readSettings()
{
    // Load previous settings based on organization name and application name (set in main.cpp)
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    // Load the geometry from the settings file. Value is saved under "geometry" key
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();

    // If the geometry does not exist (A.K.A first time running application), it sets the geometry manually
    // Otherwise, restore the geometry that exists already
    if (geometry.isEmpty())
    {
        // Get current desktop screen size
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);

        // Set the window size to 80% of width and 90% of height. Move window to middle
        resize(availableGeometry.width() * 0.8, availableGeometry.height() * 0.9);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    }
    else
    {
        restoreGeometry(geometry);
    }

    windowViewType = (WindowViewType)settings.value("windowViewType", "0").toInt();
    if (windowViewType < WindowViewType::AxialCoronalLoRes || windowViewType > WindowViewType::AxialCoronalHiRes)
    {
        qWarning() << "Invalid window view type saved in settings: " << (int)windowViewType;
        windowViewType = WindowViewType::AxialCoronalLoRes;
    }

    defaultOpenDir = settings.value("defaultOpenDir", QDir::homePath()).toString();
    defaultSavePath = settings.value("defaultSavePath", QDir::homePath()).toString();

    lastUpdateCheck = settings.value("lastUpdateCheck", QDateTime::fromSecsSinceEpoch(1)).toDateTime();
}

void MainWindow::writeSettings()
{
    // Load previous settings based on organization name and application name (set in main.cpp)
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    // Set key "geometry" to the current window position and size
    settings.setValue("geometry", saveGeometry());

    settings.setValue("windowViewType", (int)windowViewType);

    settings.setValue("defaultOpenDir", defaultOpenDir);
    settings.setValue("defaultSavePath", defaultSavePath);

    settings.setValue("lastUpdateCheck", lastUpdateCheck);
}

void MainWindow::on_actionExit_triggered()
{
    // When exit is clicked in menu, close the application
    this->close();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox aboutBox(QMessageBox::Information, "About the Program",
                         QObject::tr("<p><span style=\"font-weight: 600;\">SIUE Fat Segmentation Tool v%1</span></p>"
                                     "<p><span style=\"font-weight: 600;\">Creator:</span> Addison Elliott</p>"
                                     "<p><span style=\"font-weight: 600;\">Release Date: </span>8/21/2017</p>"
                                     "<p><span style=\"font-weight: 600;\">Advisor:</span> Jon Klingensmith</p>"
                                     "<p><span style=\"font-weight: 600;\">School:</span> Southern Illinois University Edwardsville</p>"
                                     "<p><strong>Thanks To:&nbsp;</strong></p>"
                                     "<ul>"
                                     "<li><a href=\"https://thenounproject.com/terrence.k.oleary\" data-reactid=\".2.2:$row=10.$hero.0.$hero=1meta=13715.$hero=13715=1meta=1info.0.1.1\">Terrence Kevin Oleary</a>&nbsp;- Eraser icon in Tracing tab</li>"
                                     "<li><a href=\"https://thenounproject.com/latyshevaanastasia1\" data-reactid=\".3.2:$row=10.$hero.0.$hero=1meta=1811391.$hero=1811391=1meta=1info.0.1.1\">Anastasia Latysheva</a>&nbsp;- Pencil icon in Tracing tab</li>"
                                     "</ul>")
                         .arg(QCoreApplication::applicationVersion()),
                         QMessageBox::Ok, this);
    aboutBox.setTextFormat(Qt::RichText);

    aboutBox.exec();
}

void MainWindow::on_actionCheckForUpdates_triggered()
{
    checkForUpdates();
}

void MainWindow::switchView(WindowViewType type)
{
    ui->actionAxialCoronalLoRes->setChecked(false);
    ui->actionAxialCoronalHiRes->setChecked(false);

    switch (type)
    {
        case WindowViewType::AxialCoronalLoRes:
        {
            bool prev = this->ui->actionAxialCoronalLoRes->blockSignals(true);
            this->ui->actionAxialCoronalLoRes->setChecked(true);
            this->ui->actionAxialCoronalLoRes->blockSignals(prev);
            auto newView = new viewAxialCoronalLoRes(this, fatImage, waterImage, subConfig, tracingData);
            this->setCentralWidget(newView);
            ui->actionAxialCoronalLoRes->setChecked(true);
            newView->readSettings();
            newView->setupDefaults();
        }
        break;

        case WindowViewType::AxialCoronalHiRes:
        {
            bool prev = this->ui->actionAxialCoronalHiRes->blockSignals(true);
            this->ui->actionAxialCoronalHiRes->setChecked(true);
            this->ui->actionAxialCoronalHiRes->blockSignals(prev);
            auto newView = new viewAxialCoronalHiRes(this, fatImage, waterImage, subConfig, tracingData);
            this->setCentralWidget(newView);
            ui->actionAxialCoronalHiRes->setChecked(true);
            newView->readSettings();
            newView->setupDefaults();
        }
        break;
    }

    windowViewType = type;
}

void MainWindow::checkForUpdates()
{
    qDebug() << "Checking for updates...";

    // Send API request to update URL string to retrieve the latest release
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    // Connect a slot to be called when the request is finished
    connect(manager, SIGNAL(finished(QNetworkReply *)),
            this, SLOT(networkManager_replyFinished(QNetworkReply *)));

    // Call GET request at the specified URL
    manager->get(QNetworkRequest(QUrl(updateURLString)));
}

void MainWindow::networkManager_replyFinished(QNetworkReply *reply)
{
    // If there was an error, print it, otherwise parse the JSON and determine if an update is needed
    if (reply->error())
    {
        qInfo() << "Error while attempting to check for new updates";
        qInfo() << reply->errorString();
        reply->deleteLater();
        return;
    }

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

    // Delete reply now since we read data from it
    reply->deleteLater();

    if (document.isNull())
    {
        qInfo() << "Error while attempting to check for new updates. Failed to parse JSON response";
        return;
    }

    // Get object from document (only meant for reading/writing) while object allows you to read/write the data
    QJsonObject response = document.object();

    // Get version tag and remove the preceding v in front.
    QString versionTag = response["tag_name"].toString().remove(0, 1);
    QVersionNumber latestVersion = QVersionNumber::fromString(versionTag);

    // Get current version from APP_VERSION macro
    QVersionNumber currentVersion = QVersionNumber::fromString(APP_VERSION);

    // If latest version was not valid, then throw error and return
    if (latestVersion.isNull())
    {
        qInfo() << "Error while attempting to check for new updates. Unable to parse the version tag" << versionTag;
        return;
    }

    // Check if latest release is greater than current release
    if (latestVersion > currentVersion)
    {
        // Create regex pattern to find correct installation
        // I follow this general file naming guideline for uploading releases:
        // [Program Name]-[Version]-[Debug/Release]-[Win/OSX]_[x64/x86]
        QString pattern = "^.*-.*-";

        // For auto-update, just have user keep on Release binaries
//                    #ifdef QT_DEBUG
//                        pattern += "Debug-";
//                    #elif
            pattern += "Release-";
//                    #endif

        #ifdef Q_OS_MACOS
            // For MacOSX, we are going to use any DMG file that is present. Most processors are 64-bit so becoming less of an issue
            pattern += "OSX_.*$"
        #elif defined(Q_OS_WIN64)
            pattern += "Win_x64.*$";
        #elif defined(Q_OS_WIN32)
            pattern += "Win_x86.*$";
        #endif

        // Construct regex from the pattern
        QRegularExpression re(pattern);

        // Loop through each 'asset' which is the available downloads from URL
        QJsonArray assets = response["assets"].toArray();
        for (int i = 0; i < assets.size(); ++i)
        {
            QJsonObject asset = assets[i].toObject();

            // Get asset filename
            QString assetName = asset["name"].toString();

            // If the file name does not match, then continue
            if (!re.match(assetName).hasMatch())
                continue;

            // Ask user if they want to install the update
            if (QMessageBox::information(this, "Update Available", QString("Version %1 is available.\nDo you want to install it now?\n\n"
                    "A browser will pop up with the download. Double-click the installer to install the new version.")
                    .arg(versionTag), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            {
                // If there was an error while opening URL then inform user
                if (!QDesktopServices::openUrl(QUrl(asset["browser_download_url"].toString())))
                {
                    QMessageBox::information(this, "Error Occurred", QString("Unable to open the URL in your default browser. Please manually "
                        "install the new update by going here: %1").arg(manualURLString), QMessageBox::Ok);
                }
            }
            else
            {
                // User said they do not want to update now, update timestamp so we don't annoy them for a little bit
                lastUpdateCheck = QDateTime::currentDateTime();
                return;
            }

            // We tried to download the new update, so break the loop
            break;
        }
    }

    // Since the user is downloading update, close this instance
    this->close();

    // Update the timestamp that keeps track of last time update was checked
    lastUpdateCheck = QDateTime::currentDateTime();
}

void MainWindow::on_actionAxialCoronalLoRes_triggered(bool checked)
{
    if (checked)
        switchView(WindowViewType::AxialCoronalLoRes);
    else // Prevent user from unchecking the box without toggling to something else
        ui->actionAxialCoronalLoRes->setChecked(true);
}

void MainWindow::on_actionAxialCoronalHiRes_triggered(bool checked)
{
    if (checked)
        switchView(WindowViewType::AxialCoronalHiRes);
    else // Prevent user from unchecking the box without toggling to something else
        ui->actionAxialCoronalHiRes->setChecked(true);
}

MainWindow::~MainWindow()
{
    // Save current window settings for next time
    writeSettings();

    delete fatImage;
    delete waterImage;
    delete subConfig;
    delete tracingData;

//    delete imageZip;
    delete tracingResultsZip;

    delete ui;
    ui = nullptr;
}
