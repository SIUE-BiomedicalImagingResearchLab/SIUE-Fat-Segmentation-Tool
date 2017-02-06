#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    fatImage(new NIFTImage()), waterImage(new NIFTImage()), subConfig(new SubjectConfig()), tracingData(new TracingData())
{  
    this->fatImage->setSubjectConfig(subConfig);
    this->waterImage->setSubjectConfig(subConfig);

    // Read window settings(size of window from last time application was used) upon initialization
    readSettings();

    this->ui->setupUi(this);

    // Setup the initial view
    this->switchView(windowViewType);

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
    defaultSaveDir = settings.value("defaultSaveDir", QDir::homePath()).toString();
}

void MainWindow::writeSettings()
{
    // Load previous settings based on organization name and application name (set in main.cpp)
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    // Set key "geometry" to the current window position and size
    settings.setValue("geometry", saveGeometry());

    settings.setValue("windowViewType", (int)windowViewType);

    settings.setValue("defaultOpenDir", defaultOpenDir);
    settings.setValue("defaultSaveDir", defaultSaveDir);
}

void MainWindow::on_actionExit_triggered()
{
    // When exit is clicked in menu, close the application
    this->close();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox aboutBox(QMessageBox::Information, "About the Program",
                         QObject::tr("<p><span style=\"font-weight: 600;\">Visceral Fat Validation v%1</span></p>"
                                     "<p><span style=\"font-weight: 600;\">Creator:</span> Addison Elliott</p>"
                                     "<p><span style=\"font-weight: 600;\">Release Date: </span>2/06/2017</p>"
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
            this->setCentralWidget(new viewAxialCoronalLoRes(this, fatImage, waterImage, subConfig, tracingData));
            ui->actionAxialCoronalLoRes->setChecked(true);
        }
        break;

        case WindowViewType::AxialCoronalHiRes:
        {
            bool prev = this->ui->actionAxialCoronalHiRes->blockSignals(true);
            this->ui->actionAxialCoronalHiRes->setChecked(true);
            this->ui->actionAxialCoronalHiRes->blockSignals(prev);
            this->setCentralWidget(new viewAxialCoronalHiRes(this, fatImage, waterImage, subConfig, tracingData));
            ui->actionAxialCoronalHiRes->setChecked(true);
        }
        break;
    }

    windowViewType = type;
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

    delete ui;
    ui = nullptr;
}
