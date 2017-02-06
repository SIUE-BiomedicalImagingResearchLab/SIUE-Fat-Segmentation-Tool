#include "view_axialcoronallores.h"
#include "ui_view_axialcoronallores.h"

#include "ui_mainwindow.h"
#include "mainwindow.h"

viewAxialCoronalLoRes::viewAxialCoronalLoRes(QWidget *parent, NIFTImage *fatImage, NIFTImage *waterImage, SubjectConfig *subConfig, TracingData *tracingData) :
    QWidget(parent),
    ui(new Ui::viewAxialCoronalLoRes),
    fatImage(fatImage), waterImage(waterImage), subConfig(subConfig), tracingData(tracingData),
    undoView(NULL), undoStack(new QUndoStack(this)),
    lblStatusLocation(new QLabel(this)),

    // Home Tab Shortcuts
    upShortcut(new QShortcut(QKeySequence("up"), this)), downShortcut(new QShortcut(QKeySequence("down"), this)),
    leftShortcut(new QShortcut(QKeySequence("left"), this)), rightShortcut(new QShortcut(QKeySequence("right"), this)),

    // Axial Display Tab Shortcuts
    fatRadioBtnShortcut(new QShortcut(QKeySequence("q"), this)),
    waterRadioBtnShortcut(new QShortcut(QKeySequence("w"), this)),
    fatFracRadioBtnShortcut(new QShortcut(QKeySequence("e"), this)),
    waterFracRadioBtnShortcut(new QShortcut(QKeySequence("r"), this)),
    fatWaterRadioBtnShortcut(new QShortcut(QKeySequence("t"), this)),
    waterFatRadioBtnShortcut(new QShortcut(QKeySequence("y"), this)),

    resetViewShortcut(new QShortcut(QKeySequence("Space"), this)),

    // Tracing Tab Shortcuts
    EATRadioBtnShortcut(new QShortcut(QKeySequence("1"), this)), IMATRadioBtnShortcut(new QShortcut(QKeySequence("2"), this)),
    PAATRadioBtnShortcut(new QShortcut(QKeySequence("3"), this)), PATRadioBtnShortcut(new QShortcut(QKeySequence("4"), this)),
    SCATRadioBtnShortcut(new QShortcut(QKeySequence("5"), this)), VATRadioBtnShortcut(new QShortcut(QKeySequence("6"), this)),
    EATCheckBoxShortcut(new QShortcut(QKeySequence("Ctrl+1"), this)), IMATCheckBoxShortcut(new QShortcut(QKeySequence("Ctrl+2"), this)),
    PAATCheckBoxShortcut(new QShortcut(QKeySequence("Ctrl+3"), this)), PATCheckBoxShortcut(new QShortcut(QKeySequence("Ctrl+4"), this)),
    SCATCheckBoxShortcut(new QShortcut(QKeySequence("Ctrl+5"), this)), VATCheckBoxShortcut(new QShortcut(QKeySequence("Ctrl+6"), this)),

    drawPointsModeShortcut(new QShortcut(QKeySequence("z"), this)),
    erasePointsModeShortcut(new QShortcut(QKeySequence("x"), this))
{
    this->ui->setupUi(this);

    connect(undoStack, SIGNAL(canUndoChanged(bool)), this, SLOT(undoStack_canUndoChanged(bool)));
    connect(undoStack, SIGNAL(canRedoChanged(bool)), this, SLOT(undoStack_canRedoChanged(bool)));
    this->ui->glWidgetAxial->setUndoStack(undoStack);
    this->ui->glWidgetCoronal->setUndoStack(undoStack);

    this->ui->glWidgetAxial->setup(fatImage, waterImage, tracingData);
    this->ui->glWidgetCoronal->setup(fatImage, waterImage);

    this->parentMain()->ui->statusBar->addPermanentWidget(this->lblStatusLocation);
    this->ui->glWidgetAxial->setLocationLabel(this->lblStatusLocation);

    // Set current tab to zero in case I am on a different tab in designer.
    this->ui->settingsWidget->setCurrentIndex(0);

    if (fatImage->isLoaded() && waterImage->isLoaded())
    {
        setEnableSettings(true);
        setupDefaults();
    }
    else
        setEnableSettings(false);

    // Home Tab Shortcuts
    connect(upShortcut, SIGNAL(activated()), this, SLOT(upShortcut_triggered()));
    connect(downShortcut, SIGNAL(activated()), this, SLOT(downShortcut_triggered()));
    connect(leftShortcut, SIGNAL(activated()), this, SLOT(leftShortcut_triggered()));
    connect(rightShortcut, SIGNAL(activated()), this, SLOT(rightShortcut_triggered()));

    // Axial Display Tab Shortcuts
    connect(fatRadioBtnShortcut, SIGNAL(activated()), ui->fatRadioBtn, SLOT(toggle()));
    connect(waterRadioBtnShortcut, SIGNAL(activated()), ui->waterRadioBtn, SLOT(toggle()));
    connect(fatFracRadioBtnShortcut, SIGNAL(activated()), ui->fatFracRadioBtn, SLOT(toggle()));
    connect(waterFracRadioBtnShortcut, SIGNAL(activated()), ui->waterFracRadioBtn, SLOT(toggle()));
    connect(fatWaterRadioBtnShortcut, SIGNAL(activated()), ui->fatWaterRadioBtn, SLOT(toggle()));
    connect(waterFatRadioBtnShortcut, SIGNAL(activated()), ui->waterFatRadioBtn, SLOT(toggle()));

    connect(resetViewShortcut, SIGNAL(activated()), ui->resetViewBtn, SLOT(click()));

    // Tracing Tab Shortcuts
    connect(EATRadioBtnShortcut, SIGNAL(activated()), ui->EATRadioBtn, SLOT(toggle()));
    connect(IMATRadioBtnShortcut, SIGNAL(activated()), ui->IMATRadioBtn, SLOT(toggle()));
    connect(PAATRadioBtnShortcut, SIGNAL(activated()), ui->PAATRadioBtn, SLOT(toggle()));
    connect(PATRadioBtnShortcut, SIGNAL(activated()), ui->PATRadioBtn, SLOT(toggle()));
    connect(SCATRadioBtnShortcut, SIGNAL(activated()), ui->SCATRadioBtn, SLOT(toggle()));
    connect(VATRadioBtnShortcut, SIGNAL(activated()), ui->VATRadioBtn, SLOT(toggle()));

    connect(EATCheckBoxShortcut, SIGNAL(activated()), ui->EATCheckBox, SLOT(toggle()));
    connect(IMATCheckBoxShortcut, SIGNAL(activated()), ui->IMATCheckBox, SLOT(toggle()));
    connect(PAATCheckBoxShortcut, SIGNAL(activated()), ui->PAATCheckBox, SLOT(toggle()));
    connect(PATCheckBoxShortcut, SIGNAL(activated()), ui->PATCheckBox, SLOT(toggle()));
    connect(SCATCheckBoxShortcut, SIGNAL(activated()), ui->SCATCheckBox, SLOT(toggle()));
    connect(VATCheckBoxShortcut, SIGNAL(activated()), ui->VATCheckBox, SLOT(toggle()));

    connect(drawPointsModeShortcut, SIGNAL(activated()), ui->drawPointsBtn, SLOT(click()));
    connect(erasePointsModeShortcut, SIGNAL(activated()), ui->eraserBtn, SLOT(click()));

    // Actions
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

        parentMain()->setWindowTitle(QCoreApplication::applicationName() + " - " + path);

        // The settings box is disabled to prevent moving stuff before anything is loaded
        setEnableSettings(true);

        // Initialize the tracing data to be the same size as the image and all zeros (no traces)
        // Also initialize each layer of time to be the same size as Z dim (one for each slice)
        for (auto &layer : tracingData->layers)
            layer.load(fatImage->getXDim(), fatImage->getYDim(), fatImage->getZDim());

        ui->glWidgetAxial->imageLoaded();
        ui->glWidgetCoronal->imageLoaded();

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

void viewAxialCoronalLoRes::setEnableSettings(bool enable)
{
    ui->settingsWidget->setEnabled(enable);

    // Home Tab Shortcuts
    upShortcut->setEnabled(enable);
    downShortcut->setEnabled(enable);
    leftShortcut->setEnabled(enable);
    rightShortcut->setEnabled(enable);

    // Axial Display Tab Shortcuts
    fatRadioBtnShortcut->setEnabled(enable);
    waterRadioBtnShortcut->setEnabled(enable);
    fatFracRadioBtnShortcut->setEnabled(enable);
    waterFracRadioBtnShortcut->setEnabled(enable);
    fatWaterRadioBtnShortcut->setEnabled(enable);
    waterFatRadioBtnShortcut->setEnabled(enable);

    resetViewShortcut->setEnabled(enable);

    // Tracing Tab Shortcuts
    EATRadioBtnShortcut->setEnabled(enable);
    IMATRadioBtnShortcut->setEnabled(enable);
    PAATRadioBtnShortcut->setEnabled(enable);
    PATRadioBtnShortcut->setEnabled(enable);
    SCATRadioBtnShortcut->setEnabled(enable);
    VATRadioBtnShortcut->setEnabled(enable);

    EATCheckBoxShortcut->setEnabled(enable);
    IMATCheckBoxShortcut->setEnabled(enable);
    PAATCheckBoxShortcut->setEnabled(enable);
    PATCheckBoxShortcut->setEnabled(enable);
    SCATCheckBoxShortcut->setEnabled(enable);
    VATCheckBoxShortcut->setEnabled(enable);

    drawPointsModeShortcut->setEnabled(enable);
    erasePointsModeShortcut->setEnabled(enable);
}

void viewAxialCoronalLoRes::setupDefaults()
{
    // The default slice that it will go to is half of the zDim
    QVector4D defaultLocation = QVector4D(Location::NoChange, floor(fatImage->getYDim() / 2), floor(fatImage->getZDim() / 2), Location::NoChange);

    // In the OpenGL widget, set the default location
    ui->glWidgetAxial->setLocation(defaultLocation);
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
    // Set the current radio button based on default layer to be drawing with
    switch (ui->glWidgetAxial->getTracingLayer())
    {
        case TracingLayer::EAT: ui->EATRadioBtn->setChecked(true); break;
        case TracingLayer::IMAT: ui->IMATRadioBtn->setChecked(true); break;
        case TracingLayer::PAAT: ui->PAATRadioBtn->setChecked(true); break;
        case TracingLayer::PAT: ui->PATRadioBtn->setChecked(true); break;
        case TracingLayer::SCAT: ui->SCATRadioBtn->setChecked(true); break;
        case TracingLayer::VAT: ui->VATRadioBtn->setChecked(true); break;
    }

    switch (ui->glWidgetAxial->getDrawMode())
    {
        case DrawMode::Points: ui->drawPointsBtn->setChecked(true); break;
        case DrawMode::Erase: ui->eraserBtn->setChecked(true); break;
    }

    ui->drawModeStackedWidget->setCurrentIndex((int)ui->glWidgetAxial->getDrawMode());
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
    if (parentMain()->saveTracingResultsPath.isNull())
        actionSaveAs_triggered();
    else if (ui->glWidgetAxial->saveTracingData(parentMain()->saveTracingResultsPath, false))
        parentMain()->ui->statusBar->showMessage(QObject::tr("Successfully saved file at %1").arg(parentMain()->saveTracingResultsPath), 4000);
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
        parentMain()->saveTracingResultsPath = path;
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
        parentMain()->saveTracingResultsPath = path;
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

    // Add a BrightnessChangeCommand to change the brightness
    undoStack->push(new BrightnessChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->brightnessSlider, ui->brightnessSpinBox));
}

void viewAxialCoronalLoRes::on_brightnessSpinBox_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (value / 100.0f == ui->glWidgetAxial->getBrightness())
        return;

    // Add a BrightnessChangeCommand to change the brightness
    undoStack->push(new BrightnessChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->brightnessSlider, ui->brightnessSpinBox));
}

void viewAxialCoronalLoRes::on_brightnessThresSpinBox_valueChanged(int value)
{
    // If the new value is equal to the current value, then do nothing
    if (value / 100.0f == ui->glWidgetAxial->getBrightnessThreshold())
        return;

    // Add a BrightnessThresChangeCommand to change the brightness threshold
    undoStack->push(new BrightnessThresChangeCommand(value / 100.0f, ui->glWidgetAxial, ui->brightnessThresSpinBox));
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
    parentMain()->ui->statusBar->showMessage(QObject::tr("Change slice view to %1").arg(newBtn->text()), 4000);
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
    parentMain()->ui->statusBar->showMessage(QObject::tr("%1 EAT tracing layer").arg(checked ? "Showing" : "Hiding"), 4000);
    undoStack->push(new TracingLayerVisibleChangeCommand(TracingLayer::EAT, checked, ui->glWidgetAxial, ui->EATCheckBox));
}

void viewAxialCoronalLoRes::on_IMATCheckBox_toggled(bool checked)
{
    parentMain()->ui->statusBar->showMessage(QObject::tr("%1 IMAT tracing layer").arg(checked ? "Showing" : "Hiding"), 4000);
    undoStack->push(new TracingLayerVisibleChangeCommand(TracingLayer::IMAT, checked, ui->glWidgetAxial, ui->IMATCheckBox));
}

void viewAxialCoronalLoRes::on_PAATCheckBox_toggled(bool checked)
{
    parentMain()->ui->statusBar->showMessage(QObject::tr("%1 PAAT tracing layer").arg(checked ? "Showing" : "Hiding"), 4000);
    undoStack->push(new TracingLayerVisibleChangeCommand(TracingLayer::PAAT, checked, ui->glWidgetAxial, ui->PAATCheckBox));
}

void viewAxialCoronalLoRes::on_PATCheckBox_toggled(bool checked)
{
    parentMain()->ui->statusBar->showMessage(QObject::tr("%1 PAT tracing layer").arg(checked ? "Showing" : "Hiding"), 4000);
    undoStack->push(new TracingLayerVisibleChangeCommand(TracingLayer::PAT, checked, ui->glWidgetAxial, ui->PATCheckBox));
}

void viewAxialCoronalLoRes::on_SCATCheckBox_toggled(bool checked)
{
    parentMain()->ui->statusBar->showMessage(QObject::tr("%1 SCAT tracing layer").arg(checked ? "Showing" : "Hiding"), 4000);
    undoStack->push(new TracingLayerVisibleChangeCommand(TracingLayer::SCAT, checked, ui->glWidgetAxial, ui->SCATCheckBox));
}

void viewAxialCoronalLoRes::on_VATCheckBox_toggled(bool checked)
{
    parentMain()->ui->statusBar->showMessage(QObject::tr("%1 VAT tracing layer").arg(checked ? "Showing" : "Hiding"), 4000);
    undoStack->push(new TracingLayerVisibleChangeCommand(TracingLayer::VAT, checked, ui->glWidgetAxial, ui->VATCheckBox));
}

void viewAxialCoronalLoRes::changeDrawMode(DrawMode newMode)
{
    if (newMode == ui->glWidgetAxial->getDrawMode())
        return; // If old and new draw mode are the same, do nothing

    QPushButton *newBtn = NULL;
    QString newModeStr;
    switch (newMode)
    {
        case DrawMode::Points: newBtn = ui->drawPointsBtn; newModeStr = "Draw Points"; break;
        case DrawMode::Erase: newBtn = ui->eraserBtn; newModeStr = "Erase"; break;
    }

    QPushButton *oldBtn = NULL;
    switch (ui->glWidgetAxial->getDrawMode())
    {
        case DrawMode::Points: oldBtn = ui->drawPointsBtn; break;
        case DrawMode::Erase: oldBtn = ui->eraserBtn; break;
    }

    parentMain()->ui->statusBar->showMessage(QObject::tr("Change draw mode to %1").arg(newModeStr), 4000);
    undoStack->push(new DrawModeChangeCommand(newMode, ui->glWidgetAxial, ui->drawModeStackedWidget, oldBtn, newBtn, newModeStr));
}

void viewAxialCoronalLoRes::on_drawPointsBtn_toggled(bool checked)
{
    if (checked)
        changeDrawMode(DrawMode::Points);
}

void viewAxialCoronalLoRes::on_eraserBtn_toggled(bool checked)
{
    if (checked)
        changeDrawMode(DrawMode::Erase);
}

void viewAxialCoronalLoRes::on_eraserBrushWidthComboBox_currentIndexChanged(int index)
{
    switch (index)
    {
        case 0: ui->glWidgetAxial->setEraserBrushWidth(1); break; // 1px
        case 1: ui->glWidgetAxial->setEraserBrushWidth(2); break; // 2px
        case 2: ui->glWidgetAxial->setEraserBrushWidth(4); break; // 4px
        case 3: ui->glWidgetAxial->setEraserBrushWidth(6); break; // 6px
        case 4: ui->glWidgetAxial->setEraserBrushWidth(8); break; // 8px
        default: break;
    }
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
    // Remove location status label from the parent main status bar
    // Note: If the user exits the application, then the ui will be nullptr for parent and so
    // we need to not try and remove the widget
    if (parentMain()->ui)
        parentMain()->ui->statusBar->removeWidget(lblStatusLocation);

    if (undoView)
        delete undoView;

    // Block signals in undoStack b/c on destruction, undo/redo changed is emitted
    // and this causes an attempt to enable/disable the QAction which causes seg fault
    undoStack->blockSignals(true);
    delete undoStack;

    // Home Tab Shortcuts
    delete upShortcut;
    delete downShortcut;
    delete leftShortcut;
    delete rightShortcut;

    // Axial Display Tab Shortcuts
    delete fatRadioBtnShortcut;
    delete waterRadioBtnShortcut;
    delete fatFracRadioBtnShortcut;
    delete waterFracRadioBtnShortcut;
    delete fatWaterRadioBtnShortcut;
    delete waterFatRadioBtnShortcut;

    delete resetViewShortcut;

    // Tracing Tab Shortcuts
    delete EATRadioBtnShortcut;
    delete IMATRadioBtnShortcut;
    delete PAATRadioBtnShortcut;
    delete PATRadioBtnShortcut;
    delete SCATRadioBtnShortcut;
    delete VATRadioBtnShortcut;

    delete EATCheckBoxShortcut;
    delete IMATCheckBoxShortcut;
    delete PAATCheckBoxShortcut;
    delete PATCheckBoxShortcut;
    delete SCATCheckBoxShortcut;
    delete VATCheckBoxShortcut;

    delete drawPointsModeShortcut;
    delete erasePointsModeShortcut;

    delete ui;
}
