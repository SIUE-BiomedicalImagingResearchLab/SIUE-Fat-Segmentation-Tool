#include "view_axialcoronallores.h"
#include "ui_view_axialcoronallores.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"

viewAxialCoronalLoRes::viewAxialCoronalLoRes(QWidget *parent, NIFTImage *fatImage, NIFTImage *waterImage, SubjectConfig *subConfig) :
    QWidget(parent),
    ui(new Ui::viewAxialCoronalLoRes),
    fatImage(fatImage), waterImage(waterImage), subConfig(subConfig),
    undoView(NULL), undoStack(new QUndoStack(this)),
    EATShortcut(new QShortcut(QKeySequence("1"), this)), IMATShortcut(new QShortcut(QKeySequence("2"), this)), PAATShortcut(new QShortcut(QKeySequence("3"), this)),
    PATShortcut(new QShortcut(QKeySequence("4"), this)), SCATShortcut(new QShortcut(QKeySequence("5"), this)), VATShortcut(new QShortcut(QKeySequence("6"), this)),
    upShortcut(new QShortcut(QKeySequence("up"), this)), downShortcut(new QShortcut(QKeySequence("down"), this)),
    leftShortcut(new QShortcut(QKeySequence("left"), this)), rightShortcut(new QShortcut(QKeySequence("right"), this))
{
    this->ui->setupUi(this);

    // Disable the settingsWidget (because you cannot edit it in designer when it is disabled so I just leave
    // it enabled there). And set current tab to zero in case I am on a different tab in designer.
    this->ui->settingsWidget->setEnabled(false);
    this->ui->settingsWidget->setCurrentIndex(0);

    connect(undoStack, SIGNAL(canUndoChanged(bool)), this, SLOT(undoStack_canUndoChanged(bool)));
    connect(undoStack, SIGNAL(canRedoChanged(bool)), this, SLOT(undoStack_canRedoChanged(bool)));
    this->ui->glWidgetAxial->setUndoStack(undoStack);
    this->ui->glWidgetCoronal->setUndoStack(undoStack);

    this->parentMain()->ui->statusBar->addPermanentWidget(this->ui->lblStatusLocation);
    this->ui->glWidgetAxial->setLocationLabel(this->ui->lblStatusLocation);

    // Disable the shortcuts because the settings menu is disabled
    this->EATShortcut->setEnabled(false);
    this->IMATShortcut->setEnabled(false);
    this->PAATShortcut->setEnabled(false);
    this->PATShortcut->setEnabled(false);
    this->SCATShortcut->setEnabled(false);
    this->VATShortcut->setEnabled(false);
    this->upShortcut->setEnabled(false);
    this->downShortcut->setEnabled(false);
    this->leftShortcut->setEnabled(false);
    this->rightShortcut->setEnabled(false);

    // Connect the shortcuts to the respective radio button toggle slot
    connect(EATShortcut, SIGNAL(activated()), this, SLOT(on_EATRadioBtn_toggled()));
    connect(IMATShortcut, SIGNAL(activated()), this, SLOT(on_IMATRadioBtn_toggled()));
    connect(PAATShortcut, SIGNAL(activated()), this, SLOT(on_PAATRadioBtn_toggled()));
    connect(PATShortcut, SIGNAL(activated()), this, SLOT(on_PATRadioBtn_toggled()));
    connect(SCATShortcut, SIGNAL(activated()), this, SLOT(on_SCATRadioBtn_toggled()));
    connect(VATShortcut, SIGNAL(activated()), this, SLOT(on_VATRadioBtn_toggled()));
    connect(upShortcut, SIGNAL(activated()), this, SLOT(upShortcut_triggered()));
    connect(downShortcut, SIGNAL(activated()), this, SLOT(downShortcut_triggered()));
    connect(leftShortcut, SIGNAL(activated()), this, SLOT(leftShortcut_triggered()));
    connect(rightShortcut, SIGNAL(activated()), this, SLOT(rightShortcut_triggered()));

    connect(parentMain()->ui->actionOpen, SIGNAL(triggered()), this, SLOT(actionOpen_triggered()));
    connect(parentMain()->ui->actionSave, SIGNAL(triggered()), this, SLOT(actionSave_triggered()));
    connect(parentMain()->ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(actionSaveAs_triggered()));
    connect(parentMain()->ui->actionImportTracingData, SIGNAL(triggered()), this, SLOT(actionImportTracingData_triggered()));
    connect(parentMain()->ui->actionShow_History, SIGNAL(triggered()), this, SLOT(actionShow_History_triggered()));
    connect(parentMain()->ui->actionUndo, SIGNAL(triggered()), this, SLOT(actionUndo_triggered()));
    connect(parentMain()->ui->actionRedo, SIGNAL(triggered()), this, SLOT(actionRedo_triggered()));
}

MainWindow *viewAxialCoronalLoRes::parentMain()
{
    return static_cast<MainWindow *>(this->parent());
}

bool viewAxialCoronalLoRes::loadImage(QString path)
{
    nifti_image *fatUpperImage = NULL;
    nifti_image *fatLowerImage = NULL;
    nifti_image *waterUpperImage = NULL;
    nifti_image *waterLowerImage = NULL;

    try
    {
        qDebug() << "Loading image...";

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

        qDebug() << "Read all 4 NIFTI images";

        if (!fatImage->setImage(fatUpperImage, fatLowerImage))
            EXCEPTION("Unable to merge upper and lower image", "Unable to merge upper and lower fat images in NIFTImage class.");

        if (!waterImage->setImage(waterUpperImage, waterLowerImage))
            EXCEPTION("Unable to merge upper and lower image", "Unable to merge upper and lower water images in NIFTImage class.");

        if (!fatImage->compatible(waterImage))
            EXCEPTION("Fat and water image are incompatible", "The fat and water image are incompatible in some way. Please check the NIFTI file format of the files and try again.");

        qDebug() << "Setup fat/water images";

        setWindowTitle(QCoreApplication::applicationName() + " - " + path);

        // The settings box is disabled to prevent moving stuff before anything is loaded
        enableSettings();

        // The default slice that it will go to is half of the zDim
        QVector4D defaultLocation = QVector4D(Location::NoChange, floor(fatImage->getYDim() / 2), floor(fatImage->getZDim() / 2), Location::NoChange);
        // In the OpenGL widget, set the images and default location
        ui->glWidgetAxial->setImages(fatImage, waterImage);
        ui->glWidgetAxial->setLocation(defaultLocation);

        qDebug() << "Axial Widget images are set";

        ui->glWidgetCoronal->setImages(fatImage, waterImage);
        ui->glWidgetCoronal->setLocation(defaultLocation);

        qDebug() << "Coronal Widget images are set";

        // Setup the default controls in the GUI
        setupDefaults();

        return true;
    }
    catch (const Exception &e)
    {
        // Show a message box for this exception and free the four nifti images if they are allocated.
        // Since this is an open dialog box, we do not want to stop the application so the exception is caught here
        qWarning() << e.message();
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

    return false;
}

void viewAxialCoronalLoRes::enableSettings()
{
    // The settings box is disabled to prevent moving stuff before anything is loaded
    ui->settingsWidget->setEnabled(true);

    EATShortcut->setEnabled(true);
    IMATShortcut->setEnabled(true);
    PAATShortcut->setEnabled(true);
    PATShortcut->setEnabled(true);
    SCATShortcut->setEnabled(true);
    VATShortcut->setEnabled(true);
    upShortcut->setEnabled(true);
    downShortcut->setEnabled(true);
    leftShortcut->setEnabled(true);
    rightShortcut->setEnabled(true);
}

void viewAxialCoronalLoRes::setupDefaults()
{
    QVector4D location = ui->glWidgetAxial->getLocation();

    // -------------------------------------------- Setup Home Tab --------------------------------------------
    // Set the range of the slice spin box to be 0 to the height of the fatImage (this is upper + lower z-height)
    ui->axialSliceSpinBox->setRange(0, fatImage->getZDim() - 1);
    // Set the value of the slice spin box to be what the defaultSlice is.
    ui->axialSliceSpinBox->setValue(location.z());

    // Set the range of the slice slider to be 0 to the height of the fatImage (this is upper + lower z-height)
    ui->axialSliceSlider->setRange(0, fatImage->getZDim() - 1);
    // Set the value of the slice slider to be what the defaultSlice is.
    ui->axialSliceSlider->setValue(location.z());

    // Coronal slice spin box range is 0 to 1 minus the maximum Y dimension. Set value to default location
    ui->coronalSliceSpinBox->setRange(0, fatImage->getYDim() - 1);
    ui->coronalSliceSpinBox->setValue(location.y());

    // Coronal slice slider range is 0 to 1 minus the maximum Y dimension. Set value to default location
    ui->coronalSliceSlider->setRange(0, fatImage->getYDim() - 1);
    ui->coronalSliceSlider->setValue(location.y());

    // Saggital slice spin box range is 0 to 1 minus the maximum X dimension. Set value to default location
    ui->saggitalSliceSpinBox->setRange(0, fatImage->getXDim() - 1);
    ui->saggitalSliceSpinBox->setValue(location.x());

    // Saggital slice slider range is 0 to 1 minus the maximum X dimension. Set value to default location
    ui->saggitalSliceSlider->setRange(0, fatImage->getXDim() - 1);
    ui->saggitalSliceSlider->setValue(location.x());

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
    ui->primColorMapComboBox->setCurrentIndex((int)ui->glWidgetAxial->getPrimColorMap());
    ui->primOpacitySlider->setValue(int(ui->glWidgetAxial->getPrimOpacity() * 100.0f));
    ui->primOpacitySpinBox->setValue(int(ui->glWidgetAxial->getPrimOpacity() * 100.0f));

    // Set the secondary colormap value to the current color map and set the opacity
    ui->secdColorMapComboBox->setCurrentIndex((int)ui->glWidgetAxial->getSecdColorMap());
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
}

void viewAxialCoronalLoRes::actionOpen_triggered()
{
    // Start "Select folder" dialog at user's home path.
    QString path = QFileDialog::getExistingDirectory(this, "Open Directory", parentMain()->defaultOpenDir, QFileDialog::ShowDirsOnly);
    if (path.isNull())
        return; // If they hit cancel, do nothing

    if (loadImage(path))
    {
        // Since the NIFTI files were successfully opened, the default path in the FileChooser dialog next time will be this path
        parentMain()->defaultOpenDir = path;
        parentMain()->ui->statusBar->showMessage(QObject::tr("Successfully loaded file in %1").arg(path), 4000);
    }
}

void viewAxialCoronalLoRes::actionSave_triggered()
{
    if (saveTracingResultsPath.isNull())
        actionSaveAs_triggered();
    else if (ui->glWidgetAxial->saveTracingData(saveTracingResultsPath, false))
        parentMain()->ui->statusBar->showMessage(QObject::tr("Successfully saved file at %1").arg(saveTracingResultsPath), 4000);
}

void viewAxialCoronalLoRes::actionSaveAs_triggered()
{
    // Start dialog to select existing directory to save the tracing results
    QString path = QFileDialog::getExistingDirectory(this, "Save File in Directory", parentMain()->defaultSaveDir);
    if (path.isNull())
        return; // If they hit cancel, do nothing

    QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !fileInfo.isDir())
    {
        qWarning() << "Selected directory to save tracing results either does not exist or is not a directory: " << path;
        return;
    }

    if (ui->glWidgetAxial->saveTracingData(path))
    {
        // Since the NIFTI files were successfully saved, the default path in the dialog next time will be this path
        parentMain()->defaultSaveDir = path;
        saveTracingResultsPath = path;
        parentMain()->ui->statusBar->showMessage(QObject::tr("Successfully saved file at %1").arg(path), 4000);
    }
    else
        qWarning() << "Unable to save tracing data in path: " << path;
}

void viewAxialCoronalLoRes::actionImportTracingData_triggered()
{
    // Start dialog to select existing directory to save the tracing results
    QString path = QFileDialog::getExistingDirectory(this, "Import Data in Directory", parentMain()->defaultSaveDir);
    if (path.isNull())
        return; // If they hit cancel, do nothing

    QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !fileInfo.isDir())
    {
        qWarning() << "Selected directory to save tracing results either does not exist or is not a directory: " << path;
        return;
    }

    if (ui->glWidgetAxial->loadTracingData(path))
    {
        // Since the NIFTI files were successfully loaded, the default path in the dialog next time will be this path
        parentMain()->defaultSaveDir = path;
        saveTracingResultsPath = path;
        parentMain()->ui->statusBar->showMessage(QObject::tr("Successfully loaded tracing results in %1").arg(path), 4000);
    }
}

void viewAxialCoronalLoRes::actionUndo_triggered()
{
    parentMain()->ui->statusBar->showMessage(QObject::tr("Undid \"%1\"").arg(undoStack->undoText()), 4000);
    undoStack->undo();
}

void viewAxialCoronalLoRes::actionRedo_triggered()
{
    parentMain()->ui->statusBar->showMessage(QObject::tr("Redid \"%1\"").arg(undoStack->redoText()), 4000);
    undoStack->redo();
}

void viewAxialCoronalLoRes::actionShow_History_triggered()
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

void viewAxialCoronalLoRes::upShortcut_triggered()
{
    const int value = ui->glWidgetAxial->getLocation().y() + 1;
    if (value >= fatImage->getYDim())
        return;

    // Add a SliceChangeCommand to change the coronal slice
    undoStack->push(new LocationChangeCommand(QVector4D(Location::NoChange, value, Location::NoChange, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void viewAxialCoronalLoRes::downShortcut_triggered()
{
    const int value = ui->glWidgetAxial->getLocation().y() - 1;
    if (value < 0)
        return;

    // Add a SliceChangeCommand to change the coronal slice
    undoStack->push(new LocationChangeCommand(QVector4D(Location::NoChange, value, Location::NoChange, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void viewAxialCoronalLoRes::leftShortcut_triggered()
{
    const int value = ui->glWidgetAxial->getLocation().z() - 1;
    if (value < 0)
        return;

    // Add a SliceChangeCommand to change the axial slice
    undoStack->push(new LocationChangeCommand(QVector4D(Location::NoChange, Location::NoChange, value, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void viewAxialCoronalLoRes::rightShortcut_triggered()
{
    const int value = ui->glWidgetAxial->getLocation().z() + 1;
    if (value >= fatImage->getZDim())
        return;

    // Add a SliceChangeCommand to change the axial slice
    undoStack->push(new LocationChangeCommand(QVector4D(Location::NoChange, Location::NoChange, value, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void viewAxialCoronalLoRes::on_axialSliceSlider_valueChanged(int value)
{
    // If the new value is equal to the current slice, then do nothing
    if (value == ui->glWidgetAxial->getLocation().z())
        return;

    // Add a SliceChangeCommand to change the axial slice
    undoStack->push(new LocationChangeCommand(QVector4D(Location::NoChange, Location::NoChange, value, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void viewAxialCoronalLoRes::on_axialSliceSpinBox_valueChanged(int value)
{
    // If the new value is equal to the current slice, then do nothing
    if (value == ui->glWidgetAxial->getLocation().z())
        return;

    // Add a SliceChangeCommand to change the axial slice
    undoStack->push(new LocationChangeCommand(QVector4D(Location::NoChange, Location::NoChange, value, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void viewAxialCoronalLoRes::on_coronalSliceSlider_valueChanged(int value)
{
    // If the new value is equal to the current slice, then do nothing
    if (value == ui->glWidgetAxial->getLocation().y())
        return;

    // Add a SliceChangeCommand to change the coronal slice
    undoStack->push(new LocationChangeCommand(QVector4D(Location::NoChange, value, Location::NoChange, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void viewAxialCoronalLoRes::on_coronalSliceSpinBox_valueChanged(int value)
{
    // If the new value is equal to the current slice, then do nothing
    if (value == ui->glWidgetAxial->getLocation().y())
        return;

    // Add a SliceChangeCommand to change the coronal slice
    undoStack->push(new LocationChangeCommand(QVector4D(Location::NoChange, value, Location::NoChange, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void viewAxialCoronalLoRes::on_saggitalSliceSlider_valueChanged(int value)
{
    // If the new value is equal to the current slice, then do nothing
    if (value == ui->glWidgetAxial->getLocation().x())
        return;

    // Add a SliceChangeCommand to change the saggital slice
    undoStack->push(new LocationChangeCommand(QVector4D(value, Location::NoChange, Location::NoChange, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void viewAxialCoronalLoRes::on_saggitalSliceSpinBox_valueChanged(int value)
{
    // If the new value is equal to the current slice, then do nothing
    if (value == ui->glWidgetAxial->getLocation().x())
        return;

    // Add a SliceChangeCommand to change the saggital slice
    undoStack->push(new LocationChangeCommand(QVector4D(value, Location::NoChange, Location::NoChange, Location::NoChange), ui->glWidgetAxial, ui->glWidgetCoronal,
                                              ui->axialSliceSlider, ui->axialSliceSpinBox, ui->coronalSliceSlider,
                                              ui->coronalSliceSpinBox, ui->saggitalSliceSlider, ui->saggitalSliceSpinBox));
}

void viewAxialCoronalLoRes::on_brightnessSlider_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (value / 100.0f == ui->glWidgetAxial->getBrightness())
        return;

    qDebug() << "Brightness triggered: " << value;

    // Add a BrightnessChangeCommand to change the brightness
    undoStack->push(new BrightnessChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->brightnessSlider, ui->brightnessSpinBox));
}

void viewAxialCoronalLoRes::on_brightnessSpinBox_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (value / 100.0f == ui->glWidgetAxial->getBrightness())
        return;

    qDebug() << "Brightness spin triggered: " << value;

    // Add a BrightnessChangeCommand to change the brightness
    undoStack->push(new BrightnessChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->brightnessSlider, ui->brightnessSpinBox));
}

void viewAxialCoronalLoRes::on_contrastSlider_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (value / 100.0f == ui->glWidgetAxial->getContrast())
        return;

    // Add a ContrastChangeCommand to change the contrast
    undoStack->push(new ContrastChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->contrastSlider, ui->contrastSpinBox));
}

void viewAxialCoronalLoRes::on_contrastSpinBox_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (value / 100.0f == ui->glWidgetAxial->getContrast())
        return;

    // Add a ContrastChangeCommand to change the contrast
    undoStack->push(new ContrastChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->contrastSlider, ui->contrastSpinBox));
}

void viewAxialCoronalLoRes::on_primColorMapComboBox_currentIndexChanged(int index)
{
    // If the index is out of the acceptable bounds for the ColorMap, then do nothing
    if (index == (int)ui->glWidgetAxial->getPrimColorMap() || index < (int)ColorMap::Autumn || index >= (int)ColorMap::Count)
        return;

    // Add a ColorMapChangeCommand to change the color map
    undoStack->push(new PrimColorMapChangeCommand((ColorMap)index, ui->glWidgetAxial, ui->primColorMapComboBox));
}

void viewAxialCoronalLoRes::on_primOpacitySlider_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (value / 100.0f == ui->glWidgetAxial->getPrimOpacity())
        return;

    // Add a PrimOpacityChangeCommand to change the primary opacity
    undoStack->push(new PrimOpacityChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->primOpacitySlider, ui->primOpacitySpinBox));
}

void viewAxialCoronalLoRes::on_primOpacitySpinBox_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (value / 100.0f == ui->glWidgetAxial->getPrimOpacity())
        return;

    // Add a PrimOpacityChangeCommand to change the primary opacity
    undoStack->push(new PrimOpacityChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->primOpacitySlider, ui->primOpacitySpinBox));
}

void viewAxialCoronalLoRes::on_secdColorMapComboBox_currentIndexChanged(int index)
{
    // If the index is out of the acceptable bounds for the ColorMap, then do nothing
    if (index == (int)ui->glWidgetAxial->getSecdColorMap() || index < (int)ColorMap::Autumn || index >= (int)ColorMap::Count)
        return;

    // Add a ColorMapChangeCommand to change the color map
    undoStack->push(new SecdColorMapChangeCommand((ColorMap)index, ui->glWidgetAxial, ui->secdColorMapComboBox));
}

void viewAxialCoronalLoRes::on_secdOpacitySlider_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (value / 100.0f == ui->glWidgetAxial->getSecdOpacity())
        return;

    // Add a SecdOpacityChangeCommand to change the secondary opacity
    undoStack->push(new SecdOpacityChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->secdOpacitySlider, ui->secdOpacitySpinBox));
}

void viewAxialCoronalLoRes::on_secdOpacitySpinBox_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (value / 100.0f == ui->glWidgetAxial->getSecdOpacity())
        return;

    // Add a SecdOpacityChangeCommand to change the secondary opacity
    undoStack->push(new SecdOpacityChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->secdOpacitySlider, ui->secdOpacitySpinBox));
}

void viewAxialCoronalLoRes::changeSliceView(SliceDisplayType newType)
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

void viewAxialCoronalLoRes::on_fatRadioBtn_toggled(bool checked)
{
    if (checked)
        changeSliceView(SliceDisplayType::FatOnly);
}

void viewAxialCoronalLoRes::on_waterRadioBtn_toggled(bool checked)
{
    if (checked)
        changeSliceView(SliceDisplayType::WaterOnly);
}

void viewAxialCoronalLoRes::on_fatFracRadioBtn_toggled(bool checked)
{
    if (checked)
        changeSliceView(SliceDisplayType::FatFraction);
}

void viewAxialCoronalLoRes::on_waterFracRadioBtn_toggled(bool checked)
{
    if (checked)
        changeSliceView(SliceDisplayType::WaterFraction);
}

void viewAxialCoronalLoRes::on_fatWaterRadioBtn_toggled(bool checked)
{
    if (checked)
        changeSliceView(SliceDisplayType::FatWater);
}

void viewAxialCoronalLoRes::on_waterFatRadioBtn_toggled(bool checked)
{
    if (checked)
        changeSliceView(SliceDisplayType::WaterFat);
}

void viewAxialCoronalLoRes::on_resetViewBtn_clicked()
{
    ui->glWidgetAxial->resetView();
    ui->glWidgetCoronal->resetView();
}

void viewAxialCoronalLoRes::changeTracingLayer(TracingLayer newLayer)
{
    if (newLayer == ui->glWidgetAxial->getTracingLayer())
        return; // If old and new layer are same, do nothing

    QRadioButton *newBtn = NULL;
    switch (newLayer)
    {
        case TracingLayer::EAT: newBtn = ui->EATRadioBtn; break;
        case TracingLayer::IMAT: newBtn = ui->IMATRadioBtn; break;
        case TracingLayer::PAAT: newBtn = ui->PAATRadioBtn; break;
        case TracingLayer::PAT: newBtn = ui->PATRadioBtn; break;
        case TracingLayer::SCAT: newBtn = ui->SCATRadioBtn; break;
        case TracingLayer::VAT: newBtn = ui->VATRadioBtn; break;
    }

    QRadioButton *oldBtn = NULL;
    switch (ui->glWidgetAxial->getTracingLayer())
    {
        case TracingLayer::EAT: oldBtn = ui->EATRadioBtn; break;
        case TracingLayer::IMAT: oldBtn = ui->IMATRadioBtn; break;
        case TracingLayer::PAAT: oldBtn = ui->PAATRadioBtn; break;
        case TracingLayer::PAT: oldBtn = ui->PATRadioBtn; break;
        case TracingLayer::SCAT: oldBtn = ui->SCATRadioBtn; break;
        case TracingLayer::VAT: oldBtn = ui->VATRadioBtn; break;
    }

    parentMain()->ui->statusBar->showMessage(QObject::tr("Change tracing layer to %1").arg(newBtn->text()), 4000);
    undoStack->push(new TracingLayerChangeCommand(newLayer, ui->glWidgetAxial, oldBtn, newBtn));
}

void viewAxialCoronalLoRes::on_EATRadioBtn_toggled(bool checked)
{
    if (checked)
        changeTracingLayer(TracingLayer::EAT);
}

void viewAxialCoronalLoRes::on_IMATRadioBtn_toggled(bool checked)
{
    if (checked)
        changeTracingLayer(TracingLayer::IMAT);
}

void viewAxialCoronalLoRes::on_PAATRadioBtn_toggled(bool checked)
{
    if (checked)
        changeTracingLayer(TracingLayer::PAAT);
}

void viewAxialCoronalLoRes::on_PATRadioBtn_toggled(bool checked)
{
    if (checked)
        changeTracingLayer(TracingLayer::PAT);
}

void viewAxialCoronalLoRes::on_SCATRadioBtn_toggled(bool checked)
{
    if (checked)
        changeTracingLayer(TracingLayer::SCAT);
}

void viewAxialCoronalLoRes::on_VATRadioBtn_toggled(bool checked)
{
    if (checked)
        changeTracingLayer(TracingLayer::VAT);
}

void viewAxialCoronalLoRes::on_EATCheckBox_toggled(bool checked)
{
    undoStack->push(new TracingLayerVisibleChangeCommand(TracingLayer::EAT, checked, ui->glWidgetAxial, ui->EATCheckBox));
}

void viewAxialCoronalLoRes::on_IMATCheckBox_toggled(bool checked)
{
    undoStack->push(new TracingLayerVisibleChangeCommand(TracingLayer::IMAT, checked, ui->glWidgetAxial, ui->IMATCheckBox));
}

void viewAxialCoronalLoRes::on_PAATCheckBox_toggled(bool checked)
{
    undoStack->push(new TracingLayerVisibleChangeCommand(TracingLayer::PAAT, checked, ui->glWidgetAxial, ui->PAATCheckBox));
}

void viewAxialCoronalLoRes::on_PATCheckBox_toggled(bool checked)
{
    undoStack->push(new TracingLayerVisibleChangeCommand(TracingLayer::PAT, checked, ui->glWidgetAxial, ui->PATCheckBox));
}

void viewAxialCoronalLoRes::on_SCATCheckBox_toggled(bool checked)
{
    undoStack->push(new TracingLayerVisibleChangeCommand(TracingLayer::SCAT, checked, ui->glWidgetAxial, ui->SCATCheckBox));
}

void viewAxialCoronalLoRes::on_VATCheckBox_toggled(bool checked)
{
    undoStack->push(new TracingLayerVisibleChangeCommand(TracingLayer::VAT, checked, ui->glWidgetAxial, ui->VATCheckBox));
}

void viewAxialCoronalLoRes::undoStack_canUndoChanged(bool canUndo)
{
    parentMain()->ui->actionUndo->setEnabled(canUndo);
}

void viewAxialCoronalLoRes::undoStack_canRedoChanged(bool canRedo)
{
    parentMain()->ui->actionRedo->setEnabled(canRedo);
}

viewAxialCoronalLoRes::~viewAxialCoronalLoRes()
{
    delete fatImage;
    delete waterImage;

    if (undoView)
        delete undoView;

    // Block signals in undoStack b/c on destruction, undo/redo changed is emitted
    // and this causes an attempt to enable/disable the QAction which causes seg fault
    undoStack->blockSignals(true);
    delete undoStack;

    delete EATShortcut;
    delete IMATShortcut;
    delete PAATShortcut;
    delete PATShortcut;
    delete SCATShortcut;
    delete VATShortcut;

    delete upShortcut;
    delete downShortcut;
    delete leftShortcut;
    delete rightShortcut;

    delete ui;
}
