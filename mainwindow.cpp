#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{  
    ui->setupUi(this);

    // Read window settings(size of window from last time application was used) upon initialization
    readSettings();
}

void MainWindow::on_actionExit_triggered()
{
    // When exit is clicked in menu, close the application
    this->close();
}

void MainWindow::on_actionOpen_triggered()
{
    // Start "Select folder" dialog at user's home path.
    QString path = QFileDialog::getExistingDirectory(this, "Open Directory", QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (path.isNull())
        return; // If they hit cancel, do nothing

    // Load NIFTI file
    QString fatUpperPath = QDir(path).filePath("fatUpper.nii");
    QString fatLowerPath = QDir(path).filePath("waterLower.nii");
    QString waterUpperPath = QDir(path).filePath("fatUpper.nii");
    QString waterLowerPath = QDir(path).filePath("waterLower.nii");

    // Loops through each of the four paths and check if the .nii OR .nii.gz file exists. If not, an error is thrown and the function is returned.
    auto arr = {&fatUpperPath, &fatLowerPath, &waterUpperPath, &waterLowerPath};
    for (QString *path : arr)
    {
        if (!Util::FileExists(*path) && !Util::FileExists((*path).append(".gz")))
        {
            QString basePath = QFileInfo(*path).baseName();
            QMessageBox::critical(this, "File does not exist", "Unable to find the file " + basePath + ".nii or " + basePath + ".nii.gz in the selected folder.");
            return;
        }
    }

    // Since the four files exist, load the NIFTI files.
    nifti_image *fatUpperImage = nifti_image_read(fatUpperPath.toStdString().c_str(), true);
    nifti_image *fatLowerImage = nifti_image_read(fatLowerPath.toStdString().c_str(), true);
    nifti_image *waterUpperImage = nifti_image_read(waterUpperPath.toStdString().c_str(), true);
    nifti_image *waterLowerImage = nifti_image_read(waterLowerPath.toStdString().c_str(), true);

    // If an error occurred loading one of the NIFTI files, show an error box and return
    if (!fatUpperImage || !fatLowerImage || !waterUpperImage || !waterLowerImage)
    {
        QMessageBox::critical(this, "Unable to load NIFTI file", "An error occurred while loading one of the four NIFTI images.");
        nifti_image_free(fatUpperImage);
        nifti_image_free(fatLowerImage);
        nifti_image_free(waterUpperImage);
        nifti_image_free(waterLowerImage);
        return;
    }
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
}

void MainWindow::writeSettings()
{
    // Load previous settings based on organization name and application name (set in main.cpp)
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    // Set key "geometry" to the current window position and size
    settings.setValue("geometry", saveGeometry());
}

MainWindow::~MainWindow()
{
    // Save current window settings for next time
    writeSettings();
    delete ui;
}
