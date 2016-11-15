#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{  
    ui->setupUi(this);

    fatUpperImage = NULL;
    fatLowerImage = NULL;
    waterUpperImage = NULL;
    waterLowerImage = NULL;

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
    try
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
                EXCEPTION("File does not exist", "Unable to find the file " + basePath + ".nii or " + basePath + ".nii.gz in the selected folder.");
            }
        }

        // Since the four files exist, load the NIFTI files.
        fatUpperImage = nifti_image_read(fatUpperPath.toStdString().c_str(), true);
        fatLowerImage = nifti_image_read(fatLowerPath.toStdString().c_str(), true);
        waterUpperImage = nifti_image_read(waterUpperPath.toStdString().c_str(), true);
        waterLowerImage = nifti_image_read(waterLowerPath.toStdString().c_str(), true);

        // If an error occurred loading one of the NIFTI files, show an error box and return
        if (!fatUpperImage || !fatLowerImage || !waterUpperImage || !waterLowerImage)
            EXCEPTION("Unable to load NIFTI file", "An error occurred while loading one of the four NIFTI images.");

        // Check the NIFTI images if they are compatible with each other.
        // This meanns checking the dimensions and units on all of the images and making sure they are the same
        checkNIFTIImages();

        AxialFatSliceWidget *widget = this->findChild<AxialFatSliceWidget *>("glWidgetFat", Qt::FindChildrenRecursively);
        if (!widget)
            EXCEPTION("Unable to find widget", "An error occurred while finding the AxialFatSliceWidget glWidgetFat.");

        widget->setImages(fatUpperImage, fatLowerImage, waterUpperImage, waterLowerImage);
    }
    catch (const Exception &e)
    {
        // Show a message box for this exception and free the four nifti images if they are allocated.
        // Since this is an open dialog box, we do not want to stop the application so the exception is caught here
        QMessageBox::critical(this, e.title(), e.message());
        qCritical() << e.message();
        if (fatUpperImage) { nifti_image_free(fatUpperImage); fatUpperImage = NULL; }
        if (fatLowerImage) { nifti_image_free(fatLowerImage); fatLowerImage = NULL; }
        if (waterUpperImage) { nifti_image_free(waterUpperImage); waterUpperImage = NULL; }
        if (waterLowerImage) { nifti_image_free(waterLowerImage); waterLowerImage = NULL; }
    }
    catch (...)
    {
        // This is an exception that cannot be handled in this function and so it is rethrown to be handled elsewhere
        if (fatUpperImage) { nifti_image_free(fatUpperImage); fatUpperImage = NULL; }
        if (fatLowerImage) { nifti_image_free(fatLowerImage); fatLowerImage = NULL; }
        if (waterUpperImage) { nifti_image_free(waterUpperImage); waterUpperImage = NULL; }
        if (waterLowerImage) { nifti_image_free(waterLowerImage); waterLowerImage = NULL; }
        throw;
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

void MainWindow::checkNIFTIImages()
{
    if (!fatUpperImage || !fatLowerImage || !waterUpperImage || !waterLowerImage)
        return;

    if (fatUpperImage->dim[0] != 3 ||
        fatLowerImage->dim[0] != 3 ||
        fatUpperImage->dim[1] != fatLowerImage->dim[1] ||
        fatUpperImage->dim[2] != fatLowerImage->dim[2] ||
        fatUpperImage->pixdim[1] != fatLowerImage->pixdim[1] ||
        fatUpperImage->pixdim[2] != fatLowerImage->pixdim[2] ||
        fatUpperImage->pixdim[3] != fatLowerImage->pixdim[3] ||
        fatUpperImage->xyz_units != fatLowerImage->xyz_units ||
        fatUpperImage->datatype != fatLowerImage->datatype ||
        fatUpperImage->nbyper != fatLowerImage->nbyper)
        EXCEPTION("Fat upper and lower image dimensions mismatch", "The dimensions for the upper and lower fat images must be the same to be able to stitch them together.");

    if (waterUpperImage->dim[0] != 3 ||
        waterLowerImage->dim[0] != 3 ||
        waterUpperImage->dim[1] != waterLowerImage->dim[1] ||
        waterUpperImage->dim[2] != waterLowerImage->dim[2] ||
        waterUpperImage->dim[3] != waterLowerImage->dim[3] ||
        waterUpperImage->pixdim[1] != waterLowerImage->pixdim[1] ||
        waterUpperImage->pixdim[2] != waterLowerImage->pixdim[2] ||
        waterUpperImage->pixdim[3] != waterLowerImage->pixdim[3] ||
        waterUpperImage->xyz_units != waterLowerImage->xyz_units ||
        waterUpperImage->datatype != waterLowerImage->datatype ||
        waterUpperImage->nbyper != waterLowerImage->nbyper)
        EXCEPTION("Water upper and lower image dimensions mismatch", "The dimensions for the upper and lower water images must be the same to be able to stitch them together.");

    if (fatUpperImage->dim[1] != waterUpperImage->dim[1] ||
        fatUpperImage->dim[2] != waterUpperImage->dim[2] ||
        fatUpperImage->dim[3] != waterUpperImage->dim[3] ||
        fatUpperImage->pixdim[1] != waterUpperImage->pixdim[1] ||
        fatUpperImage->pixdim[2] != waterUpperImage->pixdim[2] ||
        fatUpperImage->pixdim[3] != waterUpperImage->pixdim[3] ||
        fatUpperImage->xyz_units != waterUpperImage->xyz_units ||
        fatUpperImage->datatype != waterUpperImage->datatype ||
        fatUpperImage->nbyper != waterUpperImage->nbyper)
        EXCEPTION("Water and fat image dimensions mismatch", "The dimensions for the fat and water images must be the same to be able to stitch them together with fatFraction and waterFraction.");

    if (nifti_datatype_to_opengl(fatUpperImage->datatype) == NULL)
        EXCEPTION("Invalid data type for NIFTI image", "The data type used for the NIFTI image is not supported in this application.");
}

MainWindow::~MainWindow()
{
    if (fatUpperImage) nifti_image_free(fatUpperImage);
    if (fatLowerImage) nifti_image_free(fatLowerImage);
    if (waterUpperImage) nifti_image_free(waterUpperImage);
    if (waterLowerImage) nifti_image_free(waterLowerImage);

    // Save current window settings for next time
    writeSettings();
    delete ui;
}
