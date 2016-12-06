#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{  
    this->ui->setupUi(this);

    // Apply keyboard shortcuts to the menu items. The benefit of using a predefined
    // key sequence is that it has the list of valid shortcuts for each platform and
    // is applied appropiately. This cannot be done through the Qt GUI Designer
    this->ui->actionOpen->setShortcuts(QKeySequence::Open);
    this->ui->actionExit->setShortcuts(QKeySequence::Close);
    this->ui->actionUndo->setShortcuts(QKeySequence::Undo);
    this->ui->actionRedo->setShortcuts(QKeySequence::Redo);

    this->fatImage = new NIFTImage();
    this->waterImage = new NIFTImage();

    this->undoView = NULL;
    this->undoStack = new QUndoStack(this);
    connect(undoStack, SIGNAL(canUndoChanged(bool)), this, SLOT(undoStack_canUndoChanged(bool)));
    connect(undoStack, SIGNAL(canRedoChanged(bool)), this, SLOT(undoStack_canRedoChanged(bool)));
    ui->glWidgetSlice->setUndoStack(undoStack);

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
        QString path = QFileDialog::getExistingDirectory(this, "Open Directory", defaultOpenDir, QFileDialog::ShowDirsOnly);
        if (path.isNull())
            return; // If they hit cancel, do nothing

        // Load NIFTI file
        QString fatUpperPath = QDir(path).filePath("fatUpper.nii");
        QString fatLowerPath = QDir(path).filePath("fatLower.nii");
        QString waterUpperPath = QDir(path).filePath("waterUpper.nii");
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

        // The settings box is disabled to prevent moving stuff before anything is loaded
        ui->settingsBox->setEnabled(true);

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
        ui->brightnessSlider->setValue(int(ui->glWidgetSlice->getBrightness() * 100.0f));
        // Set the brightness spin box value to the default brightness
        ui->brightnessSpinBox->setValue(int(ui->glWidgetSlice->getBrightness() * 100.0f));

        // Set the contrast slider value to the default contrast
        ui->contrastSlider->setValue(int(ui->glWidgetSlice->getContrast() * 100.0f));
        // Set the contrast spin box value to the default contrast
        ui->contrastSpinBox->setValue(int(ui->glWidgetSlice->getContrast() * 100.0f));

        // Set the EAT radio button for the default layer to be drawing with
        ui->EATRadioBtn->setChecked(true);

        // Read the current display type for the axial slice widget and check
        // the appropiate radio button for the current axial view
        switch (ui->glWidgetSlice->getDisplayType())
        {
            case AxialDisplayType::FatOnly: ui->fatRadioBtn->setChecked(true); break;
            case AxialDisplayType::WaterOnly: ui->waterRadioBtn->setChecked(true); break;
            case AxialDisplayType::FatFraction: ui->fatFracRadioBtn->setChecked(true); break;
            case AxialDisplayType::WaterFraction: ui->waterFracRadioBtn->setChecked(true); break;
        }

        // Since the NIFTI files were successfully opened, the default path in the FileChooser dialog next time will be this path
        defaultOpenDir = path;
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
    // If the new value is equal to the current slice, then do nothing
    if (ui->glWidgetSlice->getCurSlice() == value)
        return;

    // Add a SliceChangeCommand to change the axial slice
    undoStack->push(new SliceChangeCommand(ui->glWidgetSlice->getCurSlice(), value, ui->glWidgetSlice, ui->sliceSlider, ui->sliceSpinBox));
}

void MainWindow::on_sliceSpinBox_valueChanged(int value)
{
    // If the new value is equal to the current slice, then do nothing
    if (ui->glWidgetSlice->getCurSlice() == value)
        return;

    // Add a SliceChangeCommand to change the axial slice
    undoStack->push(new SliceChangeCommand(ui->glWidgetSlice->getCurSlice(), value, ui->glWidgetSlice, ui->sliceSlider, ui->sliceSpinBox));
}

void MainWindow::on_brightnessSlider_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (ui->glWidgetSlice->getBrightness() == value / 100.0f)
        return;

    // Add a BrightnessChangeCommand to change the brightness
    undoStack->push(new BrightnessChangeCommand(ui->glWidgetSlice->getBrightness(), value / 100.0f, ui->glWidgetSlice, ui->brightnessSlider, ui->brightnessSpinBox));
}

void MainWindow::on_brightnessSpinBox_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (ui->glWidgetSlice->getBrightness() == value / 100.0f)
        return;

    // Add a BrightnessChangeCommand to change the brightness
    undoStack->push(new BrightnessChangeCommand(ui->glWidgetSlice->getBrightness(), value / 100.0f, ui->glWidgetSlice, ui->brightnessSlider, ui->brightnessSpinBox));
}

void MainWindow::on_contrastSlider_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (ui->glWidgetSlice->getContrast() == value / 100.0f)
        return;

    // Add a ContrastChangeCommand to change the brightness
    undoStack->push(new ContrastChangeCommand(ui->glWidgetSlice->getContrast(), value / 100.0f, ui->glWidgetSlice, ui->contrastSlider, ui->contrastSpinBox));
}

void MainWindow::on_contrastSpinBox_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (ui->glWidgetSlice->getContrast() == value / 100.0f)
        return;

    // Add a ContrastChangeCommand to change the brightness
    undoStack->push(new ContrastChangeCommand(ui->glWidgetSlice->getContrast(), value / 100.0f, ui->glWidgetSlice, ui->contrastSlider, ui->contrastSpinBox));
}

void MainWindow::on_colorMapComboBox_currentIndexChanged(int index)
{
    // If the index is out of the acceptable bounds for the ColorMap, then do nothing
    if (ui->glWidgetSlice->getColorMap() == (ColorMap)index || index < (int)ColorMap::Autumn || index >= (int)ColorMap::Count)
        return;

    // Add a ColorMapChangeCommand to change the color map
    undoStack->push(new ColorMapChangeCommand(ui->glWidgetSlice->getColorMap(), (ColorMap)index, ui->glWidgetSlice, ui->colorMapComboBox));
}

void MainWindow::on_fatRadioBtn_toggled(bool checked)
{
    if (checked)
    {
        QRadioButton *oldBtn = NULL;
        switch (ui->glWidgetSlice->getDisplayType())
        {
            case AxialDisplayType::FatOnly: return; // Old state and new state are same, do nothing
            case AxialDisplayType::WaterOnly: oldBtn = ui->waterRadioBtn; break;
            case AxialDisplayType::FatFraction: oldBtn = ui->fatFracRadioBtn; break;
            case AxialDisplayType::WaterFraction: oldBtn = ui->waterFracRadioBtn; break;
        }

        undoStack->push(new SliceViewChangeCommand(ui->glWidgetSlice->getDisplayType(), AxialDisplayType::FatOnly, ui->glWidgetSlice, oldBtn, ui->fatRadioBtn));
    }
}

void MainWindow::on_waterRadioBtn_toggled(bool checked)
{
    if (checked)
    {
        QRadioButton *oldBtn = NULL;
        switch (ui->glWidgetSlice->getDisplayType())
        {
            case AxialDisplayType::FatOnly: oldBtn = ui->fatRadioBtn; break;
            case AxialDisplayType::WaterOnly: return; // Old state and new state are same, do nothing
            case AxialDisplayType::FatFraction: oldBtn = ui->fatFracRadioBtn; break;
            case AxialDisplayType::WaterFraction: oldBtn = ui->waterFracRadioBtn; break;
        }

        undoStack->push(new SliceViewChangeCommand(ui->glWidgetSlice->getDisplayType(), AxialDisplayType::WaterOnly, ui->glWidgetSlice, oldBtn, ui->waterRadioBtn));
    }
}

void MainWindow::on_fatFracRadioBtn_toggled(bool checked)
{
    if (checked)
    {
        QRadioButton *oldBtn = NULL;
        switch (ui->glWidgetSlice->getDisplayType())
        {
            case AxialDisplayType::FatOnly: oldBtn = ui->fatRadioBtn; break;
            case AxialDisplayType::WaterOnly: oldBtn = ui->waterRadioBtn; break;
            case AxialDisplayType::FatFraction: return; // Old state and new state are same, do nothing
            case AxialDisplayType::WaterFraction: oldBtn = ui->waterFracRadioBtn; break;
        }

        undoStack->push(new SliceViewChangeCommand(ui->glWidgetSlice->getDisplayType(), AxialDisplayType::FatFraction, ui->glWidgetSlice, oldBtn, ui->fatFracRadioBtn));
    }
}

void MainWindow::on_waterFracRadioBtn_toggled(bool checked)
{
    if (checked)
    {
        QRadioButton *oldBtn = NULL;
        switch (ui->glWidgetSlice->getDisplayType())
        {
            case AxialDisplayType::FatOnly: oldBtn = ui->fatRadioBtn; break;
            case AxialDisplayType::WaterOnly: oldBtn = ui->waterRadioBtn; break;
            case AxialDisplayType::FatFraction: oldBtn = ui->fatFracRadioBtn; break;
            case AxialDisplayType::WaterFraction: return; // Old state and new state are same, do nothing
        }

        undoStack->push(new SliceViewChangeCommand(ui->glWidgetSlice->getDisplayType(), AxialDisplayType::WaterFraction, ui->glWidgetSlice, oldBtn, ui->waterFracRadioBtn));
    }
}

void MainWindow::on_resetViewBtn_clicked()
{
    ui->glWidgetSlice->resetView();
}

void MainWindow::undoStack_canUndoChanged(bool canUndo)
{
    ui->actionUndo->setEnabled(canUndo);
}

void MainWindow::undoStack_canRedoChanged(bool canRedo)
{
    ui->actionRedo->setEnabled(canRedo);
}

void MainWindow::on_actionUndo_triggered()
{
    undoStack->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    undoStack->redo();
}

void MainWindow::on_actionShow_History_triggered()
{
    // If the undoView does not exist or is not visible, then create a new undoView to display
    if (!undoView || !undoView->isVisible())
    {
        // If the old one exists, then delete it and create a new one
        if (undoView)
        {
            undoView->close();
            delete undoView;
        }

        // Create new QUndoView based on undoStack
        undoView = new QUndoView(undoStack, this);
        undoView->setWindowFlags(Qt::Tool);

        undoView->setWindowTitle(tr("History"));
        undoView->setWindowModality(Qt::NonModal);
        undoView->show();
        undoView->setAttribute( Qt::WA_QuitOnClose, false );
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

    defaultOpenDir = settings.value("defaultDir", QDir::homePath()).toString();
}

void MainWindow::writeSettings()
{
    // Load previous settings based on organization name and application name (set in main.cpp)
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    // Set key "geometry" to the current window position and size
    settings.setValue("geometry", saveGeometry());

    settings.setValue("defaultDir", defaultOpenDir);
}

MainWindow::~MainWindow()
{
    delete fatImage;
    delete waterImage;

    if (undoView)
        delete undoView;

    //undoStack->clear();
    delete undoStack;

    // Save current window settings for next time
    writeSettings();
    delete ui;
}
