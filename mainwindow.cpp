#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{  
    // Read window settings(size of window from last time application was used) upon initialization
    readSettings();

    this->ui->setupUi(this);

    switch (windowViewType)
    {
        case WindowViewType::AxialCoronalLoRes:
        {
            bool prev = this->ui->actionAxialCoronalLoRes->blockSignals(true);
            this->ui->actionAxialCoronalLoRes->setChecked(true);
            this->ui->actionAxialCoronalLoRes->blockSignals(prev);
            this->setCentralWidget(new viewAxialCoronalLoRes(this));
        }
        break;

        case WindowViewType::AxialCoronalHiRes:
        {
            bool prev = this->ui->actionAxialCoronalHiRes->blockSignals(true);
            this->ui->actionAxialCoronalHiRes->setChecked(true);
            this->ui->actionAxialCoronalHiRes->blockSignals(prev);
            this->setCentralWidget(new viewAxialCoronalHiRes(this));
         }
         break;
    }

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
                         QObject::tr("<html><head/><body><p><span style=\" font-weight:600;\">Visceral Fat Validation v%1</span></p><p><span style=\" font-weight:600;\">Creator:</span> Addison Elliott</p><p><span style=\" font-weight:600;\">Release Date: </span>1/19/2017</p><p><span style=\" font-weight:600;\">Advisor:</span> Jon Klingensmith</p><p><span style=\" font-weight:600;\">School:</span> Southern Illinois University Edwardsville</p></body></html>")
                         .arg(QCoreApplication::applicationVersion()),
                         QMessageBox::Ok, this);
    aboutBox.setTextFormat(Qt::RichText);

    aboutBox.exec();
}

void MainWindow::switchView(WindowViewType type, bool showPrompt)
{
    ui->actionAxialCoronalLoRes->setChecked(false);
    ui->actionAxialCoronalLoRes->setChecked(false);

    switch (type)
    {
        case WindowViewType::AxialCoronalLoRes:
            ui->actionAxialCoronalLoRes->setChecked(true);
            break;

        case WindowViewType::AxialCoronalHiRes:
            ui->actionAxialCoronalHiRes->setChecked(true);
            break;
    }

    if (showPrompt)
        QMessageBox::information(this, "Switch Views", "Your current view type was acknowledged. Please restart the application to see the changes.", QMessageBox::Ok);
    windowViewType = type;
}

void MainWindow::on_actionAxialCoronalLoRes_triggered(bool checked)
{
    if (checked)
        switchView(WindowViewType::AxialCoronalLoRes);
}

void MainWindow::on_actionAxialCoronalHiRes_triggered(bool checked)
{
    if (checked)
        switchView(WindowViewType::AxialCoronalHiRes);
}

MainWindow::~MainWindow()
{
    // Save current window settings for next time
    writeSettings();

    delete ui;
}
