#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{  
    ui->setupUi(this);

    fatImage = new NIFTImage();
    waterImage = new NIFTImage();

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
    nifti_image *fatUpperImage = NULL;
    nifti_image *fatLowerImage = NULL;
    nifti_image *waterUpperImage = NULL;
    nifti_image *waterLowerImage = NULL;

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

        if (!fatImage->setImage(fatUpperImage, fatLowerImage))
            EXCEPTION("Unable to merge upper and lower image", "Unable to merge upper and lower fat images in NIFTImage class.");

        if (!waterImage->setImage(waterUpperImage, waterLowerImage))
            EXCEPTION("Unable to merge upper and lower image", "Unable to merge upper and lower water images in NIFTImage class.");

        if (!fatImage->compatible(waterImage))
            EXCEPTION("Fat and water image are incompatible", "The fat and water image are incompatible in some way. Please check the NIFTI file format of the files and try again.");

        // The default slice that it will go to is half of the zDim
        int defaultSlice = floor(fatImage->getZDim() / 2);
        // In the OpenGL widget, set the images and default axial slice
        ui->glWidgetSlice->setImages(fatImage, waterImage);
        ui->glWidgetSlice->setAxialSlice(defaultSlice);

        // Set the range of the slice spin box to be 0 to the height of the fatImage (this is upper + lower z-height
        ui->sliceSpinBox->setRange(0, fatImage->getZDim());
        // Set the value of the slice spin box to be what the defaultSlice is.
        ui->sliceSpinBox->setValue(defaultSlice);

        // Set the range of the slice slider to be 0 to the height of the fatImage (this is upper + lower z-height
        ui->sliceSlider->setRange(0, fatImage->getZDim());
        // Set the value of the slice slider to be what the defaultSlice is.
        ui->sliceSlider->setValue(defaultSlice);

        // Set the colormap value in the combo box to the current color map
        ui->colorMapComboBox->setCurrentIndex(ui->glWidgetSlice->getColorMap());

        // Set the brightness slider value to the default brightness
        ui->brightnessSlider->setValue(floor(ui->glWidgetSlice->getBrightness() * 100.0f));
        // Set the brightness spin box value to the default brightness
        ui->brightnessSpinBox->setValue(floor(ui->glWidgetSlice->getBrightness() * 100.0f));

        // Set the contrast slider value to the default contrast
        ui->contrastSlider->setValue(floor(ui->glWidgetSlice->getContrast() * 100.0f));
        // Set the contrast spin box value to the default contrast
        ui->contrastSpinBox->setValue(floor(ui->glWidgetSlice->getContrast() * 100.0f));
    }
    catch (const Exception &e)
    {
        // Show a message box for this exception and free the four nifti images if they are allocated.
        // Since this is an open dialog box, we do not want to stop the application so the exception is caught here
        QMessageBox::critical(this, e.title(), e.message());
        qCritical() << e.message();
        if (fatUpperImage) nifti_image_free(fatUpperImage);
        if (fatLowerImage) nifti_image_free(fatLowerImage);
        if (waterUpperImage) nifti_image_free(waterUpperImage);
        if (waterLowerImage) nifti_image_free(waterLowerImage);
    }
    catch (...)
    {
        // This is an exception that cannot be handled in this function and so it is rethrown to be handled elsewhere
        if (fatUpperImage) nifti_image_free(fatUpperImage);
        if (fatLowerImage) nifti_image_free(fatLowerImage);
        if (waterUpperImage) nifti_image_free(waterUpperImage);
        if (waterLowerImage) nifti_image_free(waterLowerImage);
        throw;
    }
}

void MainWindow::on_sliceSlider_valueChanged(int value)
{
    // Update the text box beside the slider. Keep the values consistent
    ui->sliceSpinBox->setValue(value);
    // Set the axial slice for the OpenGL that renders the slice
    ui->glWidgetSlice->setAxialSlice(value);
}

void MainWindow::on_sliceSpinBox_valueChanged(int value)
{
    // Update the slider beside the text box. Keep the values consistent
    ui->sliceSlider->setValue(value);
    // Set the axial slice for the OpenGL that renders the slice
    ui->glWidgetSlice->setAxialSlice(value);
}

void MainWindow::on_brightnessSlider_valueChanged(int value)
{
    // Update the text box beside the slider. Keep the values consistent
    ui->brightnessSpinBox->setValue(value);
    // Set the brightness for the AxialSliceWidget
    ui->glWidgetSlice->setBrightness(value / 100.0f);
}

void MainWindow::on_brightnessSpinBox_valueChanged(int value)
{
    // Update the slider beside the text box. Keep the values consistent
    ui->brightnessSlider->setValue(value);
    // Set the brightness for the AxialSliceWidget
    ui->glWidgetSlice->setBrightness(value / 100.0f);
}

void MainWindow::on_contrastSlider_valueChanged(int value)
{
    // Update the text box beside the slider. Keep the values consistent
    ui->contrastSpinBox->setValue(value);
    // Set the contrast for the AxialSliceWidget
    ui->glWidgetSlice->setContrast(value / 100.0f);
}

void MainWindow::on_contrastSpinBox_valueChanged(int value)
{
    // Update the slider beside the text box. Keep the values consistent
    ui->contrastSlider->setValue(value);
    // Set the contrast for the AxialSliceWidget
    ui->glWidgetSlice->setContrast(value / 100.0f);
}

void MainWindow::on_colorMapComboBox_currentIndexChanged(int index)
{
    // If the index is out of the acceptable bounds for the ColorMap, then do nothing
    if (index < (int)ColorMap::Autumn || index >= (int)ColorMap::Count)
        return;

    ui->glWidgetSlice->setColorMap((ColorMap)index);
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
    delete fatImage;
    delete waterImage;

    // Save current window settings for next time
    writeSettings();
    delete ui;
}
