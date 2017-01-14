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

    // Disable the settingsWidget (because you cannot edit it in designer when it is disabled so I just leave
    // it enabled there). And set current tab to zero in case I am on a different tab in designer.
    this->ui->settingsWidget->setEnabled(false);
    this->ui->settingsWidget->setCurrentIndex(0);

    this->subConfig = new SubjectConfig();
    this->fatImage = new NIFTImage(subConfig);
    this->waterImage = new NIFTImage(subConfig);

    this->undoView = NULL;
    this->undoStack = new QUndoStack(this);
    connect(undoStack, SIGNAL(canUndoChanged(bool)), this, SLOT(undoStack_canUndoChanged(bool)));
    connect(undoStack, SIGNAL(canRedoChanged(bool)), this, SLOT(undoStack_canRedoChanged(bool)));
    ui->glWidgetAxial->setUndoStack(undoStack);
    ui->glWidgetCoronal->setUndoStack(undoStack);

    // Read window settings(size of window from last time application was used) upon initialization
    readSettings();
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

    this->resize(1024, 768);

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
            if (!util::fileExists(*path) && !util::fileExists((*path).append(".gz")))
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

        QString configPath = QDir(path).filePath("config.xml");
        if (!util::fileExists(configPath))
            EXCEPTION("File does not exist", "Unable to find the config file config.xml");

        if (!subConfig->load(configPath))
            EXCEPTION("Error loading subject configuration", "There was an error while loading the subject configuration at " + configPath);

        // If an error occurred loading one of the NIFTI files, show an error box and return
        if (!fatUpperImage || !fatLowerImage || !waterUpperImage || !waterLowerImage)
            EXCEPTION("Unable to load NIFTI file", "An error occurred while loading one of the four NIFTI images.");

        if (!fatImage->setImage(fatUpperImage, fatLowerImage))
            EXCEPTION("Unable to merge upper and lower image", "Unable to merge upper and lower fat images in NIFTImage class.");

        if (!waterImage->setImage(waterUpperImage, waterLowerImage))
            EXCEPTION("Unable to merge upper and lower image", "Unable to merge upper and lower water images in NIFTImage class.");

        if (!fatImage->compatible(waterImage))
            EXCEPTION("Fat and water image are incompatible", "The fat and water image are incompatible in some way. Please check the NIFTI file format of the files and try again.");

        setWindowTitle(QCoreApplication::applicationName() + " - " + path);

        // The settings box is disabled to prevent moving stuff before anything is loaded
        ui->settingsWidget->setEnabled(true);

        // The default slice that it will go to is half of the zDim
        QVector4D defaultLocation = QVector4D(Location::NoChange, floor(fatImage->getYDim() / 2), floor(fatImage->getZDim() / 2), Location::NoChange);
        // In the OpenGL widget, set the images and default location
        ui->glWidgetAxial->setImages(fatImage, waterImage);
        ui->glWidgetAxial->setLocation(defaultLocation);

        ui->glWidgetCoronal->setImages(fatImage, waterImage);
        ui->glWidgetCoronal->setLocation(defaultLocation);

        // -------------------------------------------- Setup Home Tab --------------------------------------------
        // Set the range of the slice spin box to be 0 to the height of the fatImage (this is upper + lower z-height)
        ui->axialSliceSpinBox->setRange(0, fatImage->getZDim() - 1);
        // Set the value of the slice spin box to be what the defaultSlice is.
        ui->axialSliceSpinBox->setValue(defaultLocation.z());

        // Set the range of the slice slider to be 0 to the height of the fatImage (this is upper + lower z-height)
        ui->axialSliceSlider->setRange(0, fatImage->getZDim() - 1);
        // Set the value of the slice slider to be what the defaultSlice is.
        ui->axialSliceSlider->setValue(defaultLocation.z());

        // Coronal slice spin box range is 0 to 1 minus the maximum Y dimension. Set value to default location
        ui->coronalSliceSpinBox->setRange(0, fatImage->getYDim() - 1);
        ui->coronalSliceSpinBox->setValue(defaultLocation.y());

        // Coronal slice slider range is 0 to 1 minus the maximum Y dimension. Set value to default location
        ui->coronalSliceSlider->setRange(0, fatImage->getYDim() - 1);
        ui->coronalSliceSlider->setValue(defaultLocation.y());

        // Saggital slice spin box range is 0 to 1 minus the maximum X dimension. Set value to default location
        ui->saggitalSliceSpinBox->setRange(0, fatImage->getXDim() - 1);
        ui->saggitalSliceSpinBox->setValue(defaultLocation.x());

        // Saggital slice slider range is 0 to 1 minus the maximum X dimension. Set value to default location
        ui->saggitalSliceSlider->setRange(0, fatImage->getXDim() - 1);
        ui->saggitalSliceSlider->setValue(defaultLocation.x());

        // ---------------------------------------- Setup Axial Display Tab ----------------------------------------
        // Set the brightness slider value to the default brightness
        ui->brightnessSlider->setValue(int(ui->glWidgetAxial->getBrightness() * 100.0f));
        // Set the brightness spin box value to the default brightness
        ui->brightnessSpinBox->setValue(int(ui->glWidgetAxial->getBrightness() * 100.0f));

        // Set the contrast slider value to the default contrast
        ui->contrastSlider->setValue(int(ui->glWidgetAxial->getContrast() * 100.0f));
        // Set the contrast spin box value to the default contrast
        ui->contrastSpinBox->setValue(int(ui->glWidgetAxial->getContrast() * 100.0f));

        // Set the primary colormap value to the current color map and set the opacity
        ui->primColorMapComboBox->setCurrentIndex(ui->glWidgetAxial->getPrimColorMap());
        ui->primOpacitySlider->setValue(int(ui->glWidgetAxial->getPrimOpacity() * 100.0f));
        ui->primOpacitySpinBox->setValue(int(ui->glWidgetAxial->getPrimOpacity() * 100.0f));

        // Set the secondary colormap value to the current color map and set the opacity
        ui->secdColorMapComboBox->setCurrentIndex(ui->glWidgetAxial->getSecdColorMap());
        ui->secdOpacitySlider->setValue(int(ui->glWidgetAxial->getSecdOpacity() * 100.0f));
        ui->secdOpacitySpinBox->setValue(int(ui->glWidgetAxial->getSecdOpacity() * 100.0f));

        // Read the current display type for the axial slice widget and check
        // the appropiate radio button for the current axial view
        switch (ui->glWidgetAxial->getDisplayType())
        {
            case SliceDisplayType::FatOnly: ui->fatRadioBtn->setChecked(true); break;
            case SliceDisplayType::WaterOnly: ui->waterRadioBtn->setChecked(true); break;
            case SliceDisplayType::FatFraction: ui->fatFracRadioBtn->setChecked(true); break;
            case SliceDisplayType::WaterFraction: ui->waterFracRadioBtn->setChecked(true); break;
            case SliceDisplayType::FatWater: ui->fatWaterRadioBtn->setChecked(true); break;
            case SliceDisplayType::WaterFat: ui->waterFatRadioBtn->setChecked(true); break;
        }

        // Enable the secondary image box if FatWater or WaterFat selected, otherwise disable
        ui->secondaryImageBox->setEnabled((ui->glWidgetAxial->getDisplayType() == SliceDisplayType::FatWater || ui->glWidgetAxial->getDisplayType() == SliceDisplayType::WaterFat));

        // ------------------------------------------- Setup Tracing Tab -------------------------------------------
        // Set the EAT radio button for the default layer to be drawing with
        ui->EATRadioBtn->setChecked(true);

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

void MainWindow::on_axialSliceSlider_valueChanged(int value)
{
    // If the new value is equal to the current slice, then do nothing
    if (ui->glWidgetAxial->getLocation().z() == value)
        return;

    // Add a SliceChangeCommand to change the axial slice
    undoStack->push(new LocationChangeCommand(QVector4D(Location::NoChange, Location::NoChange, value, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void MainWindow::on_axialSliceSpinBox_valueChanged(int value)
{
    // If the new value is equal to the current slice, then do nothing
    if (ui->glWidgetAxial->getLocation().z() == value)
        return;

    // Add a SliceChangeCommand to change the axial slice
    undoStack->push(new LocationChangeCommand(QVector4D(Location::NoChange, Location::NoChange, value, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void MainWindow::on_coronalSliceSlider_valueChanged(int value)
{
    // If the new value is equal to the current slice, then do nothing
    if (ui->glWidgetAxial->getLocation().y() == value)
        return;

    // Add a SliceChangeCommand to change the coronal slice
    undoStack->push(new LocationChangeCommand(QVector4D(Location::NoChange, value, Location::NoChange, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void MainWindow::on_coronalSliceSpinBox_valueChanged(int value)
{
    // If the new value is equal to the current slice, then do nothing
    if (ui->glWidgetAxial->getLocation().y() == value)
        return;

    // Add a SliceChangeCommand to change the coronal slice
    undoStack->push(new LocationChangeCommand(QVector4D(Location::NoChange, value, Location::NoChange, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void MainWindow::on_saggitalSliceSlider_valueChanged(int value)
{
    // If the new value is equal to the current slice, then do nothing
    if (ui->glWidgetAxial->getLocation().x() == value)
        return;

    // Add a SliceChangeCommand to change the saggital slice
    undoStack->push(new LocationChangeCommand(QVector4D(value, Location::NoChange, Location::NoChange, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void MainWindow::on_saggitalSliceSpinBox_valueChanged(int value)
{
    // If the new value is equal to the current slice, then do nothing
    if (ui->glWidgetAxial->getLocation().x() == value)
        return;

    // Add a SliceChangeCommand to change the saggital slice
    undoStack->push(new LocationChangeCommand(QVector4D(value, Location::NoChange, Location::NoChange, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void MainWindow::on_brightnessSlider_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (ui->glWidgetAxial->getBrightness() == value / 100.0f)
        return;

    // Add a BrightnessChangeCommand to change the brightness
    undoStack->push(new BrightnessChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->brightnessSlider, ui->brightnessSpinBox));
}

void MainWindow::on_brightnessSpinBox_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (ui->glWidgetAxial->getBrightness() == value / 100.0f)
        return;

    // Add a BrightnessChangeCommand to change the brightness
    undoStack->push(new BrightnessChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->brightnessSlider, ui->brightnessSpinBox));
}

void MainWindow::on_contrastSlider_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (ui->glWidgetAxial->getContrast() == value / 100.0f)
        return;

    // Add a ContrastChangeCommand to change the contrast
    undoStack->push(new ContrastChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->contrastSlider, ui->contrastSpinBox));
}

void MainWindow::on_contrastSpinBox_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (ui->glWidgetAxial->getContrast() == value / 100.0f)
        return;

    // Add a ContrastChangeCommand to change the contrast
    undoStack->push(new ContrastChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->contrastSlider, ui->contrastSpinBox));
}

void MainWindow::on_primColorMapComboBox_currentIndexChanged(int index)
{
    // If the index is out of the acceptable bounds for the ColorMap, then do nothing
    if (ui->glWidgetAxial->getPrimColorMap() == (ColorMap)index || index < (int)ColorMap::Autumn || index >= (int)ColorMap::Count)
        return;

    // Add a ColorMapChangeCommand to change the color map
    undoStack->push(new PrimColorMapChangeCommand((ColorMap)index, ui->glWidgetAxial, ui->primColorMapComboBox));
}

void MainWindow::on_primOpacitySlider_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (ui->glWidgetAxial->getPrimOpacity() == value / 100.0f)
        return;

    // Add a PrimOpacityChangeCommand to change the primary opacity
    undoStack->push(new PrimOpacityChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->primOpacitySlider, ui->primOpacitySpinBox));
}

void MainWindow::on_primOpacitySpinBox_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (ui->glWidgetAxial->getPrimOpacity() == value / 100.0f)
        return;

    // Add a PrimOpacityChangeCommand to change the primary opacity
    undoStack->push(new PrimOpacityChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->primOpacitySlider, ui->primOpacitySpinBox));
}

void MainWindow::on_secdColorMapComboBox_currentIndexChanged(int index)
{
    // If the index is out of the acceptable bounds for the ColorMap, then do nothing
    if (ui->glWidgetAxial->getSecdColorMap() == (ColorMap)index || index < (int)ColorMap::Autumn || index >= (int)ColorMap::Count)
        return;

    // Add a ColorMapChangeCommand to change the color map
    undoStack->push(new SecdColorMapChangeCommand((ColorMap)index, ui->glWidgetAxial, ui->secdColorMapComboBox));
}

void MainWindow::on_secdOpacitySlider_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (ui->glWidgetAxial->getSecdOpacity() == value / 100.0f)
        return;

    // Add a SecdOpacityChangeCommand to change the secondary opacity
    undoStack->push(new SecdOpacityChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->secdOpacitySlider, ui->secdOpacitySpinBox));
}

void MainWindow::on_secdOpacitySpinBox_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (ui->glWidgetAxial->getSecdOpacity() == value / 100.0f)
        return;

    // Add a SecdOpacityChangeCommand to change the secondary opacity
    undoStack->push(new SecdOpacityChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->secdOpacitySlider, ui->secdOpacitySpinBox));
}

void MainWindow::changeSliceView(SliceDisplayType newType)
{
    if (newType == ui->glWidgetAxial->getDisplayType())
        return; // If old and new type are same, do nothing

    QRadioButton *newBtn = NULL;
    switch (newType)
    {
        case SliceDisplayType::FatOnly: newBtn = ui->fatRadioBtn; break;
        case SliceDisplayType::WaterOnly: newBtn = ui->waterRadioBtn; break;
        case SliceDisplayType::FatFraction: newBtn = ui->fatFracRadioBtn; break;
        case SliceDisplayType::WaterFraction: newBtn = ui->waterFracRadioBtn; break;
        case SliceDisplayType::FatWater: newBtn = ui->fatWaterRadioBtn; break;
        case SliceDisplayType::WaterFat: newBtn = ui->waterFatRadioBtn; break;
    }

    QRadioButton *oldBtn = NULL;
    switch (ui->glWidgetAxial->getDisplayType())
    {
        case SliceDisplayType::FatOnly: oldBtn = ui->fatRadioBtn; break;
        case SliceDisplayType::WaterOnly: oldBtn = ui->waterRadioBtn; break;
        case SliceDisplayType::FatFraction: oldBtn = ui->fatFracRadioBtn; break;
        case SliceDisplayType::WaterFraction: oldBtn = ui->waterFracRadioBtn; break;
        case SliceDisplayType::FatWater: oldBtn = ui->fatWaterRadioBtn; break;
        case SliceDisplayType::WaterFat: oldBtn = ui->waterFatRadioBtn; break;
    }

    // Enable the secondary image box if FatWater or WaterFat selected, otherwise disable
    ui->secondaryImageBox->setEnabled((newType == SliceDisplayType::FatWater || newType == SliceDisplayType::WaterFat));

    undoStack->push(new SliceViewChangeCommand(newType, ui->glWidgetAxial, oldBtn, newBtn));
}

void MainWindow::on_fatRadioBtn_toggled(bool checked)
{
    if (checked)
        changeSliceView(SliceDisplayType::FatOnly);
}

void MainWindow::on_waterRadioBtn_toggled(bool checked)
{
    if (checked)
        changeSliceView(SliceDisplayType::WaterOnly);
}

void MainWindow::on_fatFracRadioBtn_toggled(bool checked)
{
    if (checked)
        changeSliceView(SliceDisplayType::FatFraction);
}

void MainWindow::on_waterFracRadioBtn_toggled(bool checked)
{
    if (checked)
        changeSliceView(SliceDisplayType::WaterFraction);
}

void MainWindow::on_fatWaterRadioBtn_toggled(bool checked)
{
    if (checked)
        changeSliceView(SliceDisplayType::FatWater);
}

void MainWindow::on_waterFatRadioBtn_toggled(bool checked)
{
    if (checked)
        changeSliceView(SliceDisplayType::WaterFat);
}

void MainWindow::on_resetViewBtn_clicked()
{
    ui->glWidgetAxial->resetView();
    ui->glWidgetCoronal->resetView();
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
        undoView->setAttribute(Qt::WA_QuitOnClose, false);
    }
}

MainWindow::~MainWindow()
{
    delete fatImage;
    delete waterImage;

    if (undoView)
        delete undoView;

    delete undoStack;

    // Save current window settings for next time
    writeSettings();
    delete ui;
}
