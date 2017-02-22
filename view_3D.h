#ifndef VIEW_AXIALCORONALHIRES_H
#define VIEW_AXIALCORONALHIRES_H

#include <QWidget>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QDesktopWidget>
#include <QUndoStack>
#include <QUndoView>
#include <QVector4D>
#include <QWhatsThis>
#include <QShortcut>

#include <opencv2/opencv.hpp>

#include <nifti/nifti1.h>
#include <nifti/nifti1_io.h>

#include "displayinfo.h"
#include "niftimage.h"
#include "util.h"
#include "tracing.h"
#include "exception.h"
#include "subjectconfig.h"

namespace Ui {
class view3D;
}

class MainWindow;

class view3D : public QWidget
{
    Q_OBJECT

private:
    Ui::view3D *ui;

    NIFTImage *fatImage;
    NIFTImage *waterImage;
    SubjectConfig *subConfig;
    TracingData *tracingData;

    QUndoView *undoView;
    QUndoStack *undoStack;

    QLabel *lblStatusLocation;

    // Home Tab Shortcuts
    QShortcut *upShortcut;
    QShortcut *downShortcut;
    QShortcut *leftShortcut;
    QShortcut *rightShortcut;

    // Axial Display Tab Shortcuts
    QShortcut *fatRadioBtnShortcut;
    QShortcut *waterRadioBtnShortcut;
    QShortcut *fatFracRadioBtnShortcut;
    QShortcut *waterFracRadioBtnShortcut;
    QShortcut *fatWaterRadioBtnShortcut;
    QShortcut *waterFatRadioBtnShortcut;

    QShortcut *resetViewShortcut;

    // Tracing Tab Shortcuts
    QShortcut *EATRadioBtnShortcut;
    QShortcut *IMATRadioBtnShortcut;
    QShortcut *PAATRadioBtnShortcut;
    QShortcut *PATRadioBtnShortcut;
    QShortcut *SCATRadioBtnShortcut;
    QShortcut *VATRadioBtnShortcut;

    QShortcut *EATCheckBoxShortcut;
    QShortcut *IMATCheckBoxShortcut;
    QShortcut *PAATCheckBoxShortcut;
    QShortcut *PATCheckBoxShortcut;
    QShortcut *SCATCheckBoxShortcut;
    QShortcut *VATCheckBoxShortcut;

    QShortcut *drawPointsModeShortcut;
    QShortcut *erasePointsModeShortcut;

public:
    explicit view3D(QWidget *parent, NIFTImage *fatImage, NIFTImage *waterImage, SubjectConfig *subConfig, TracingData *tracingData);
    ~view3D();

    MainWindow *parentMain();

    bool loadImage(QString path);
    void setEnableSettings(bool enable);
    void setupDefaults();

    void readSettings();
    void writeSettings();

    void changeSliceView(SliceDisplayType newType);
    void changeTracingLayer(TracingLayer newLayer);
    void changeDrawMode(DrawMode newMode);

private slots:
    // Slots that do not contain the on_ prefix are not automatically connected with MOC and so must not include the on_
    // or they will search for items in the GUI named that. If on_ prefixed, then they are automatically connected by MOC
    void actionOpen_triggered();

    void actionSave_triggered();
    void actionSaveAs_triggered();

    void actionImportTracingData_triggered();

    void actionShow_History_triggered();
    void actionUndo_triggered();
    void actionRedo_triggered();

    void upShortcut_triggered();
    void downShortcut_triggered();
    void leftShortcut_triggered();
    void rightShortcut_triggered();

    void on_axialSliceSlider_valueChanged(int value);
    void on_axialSliceSpinBox_valueChanged(int value);

    void on_coronalSliceSlider_valueChanged(int value);
    void on_coronalSliceSpinBox_valueChanged(int value);

    void on_saggitalSliceSlider_valueChanged(int value);
    void on_saggitalSliceSpinBox_valueChanged(int value);

    void on_brightnessSlider_valueChanged(int value);
    void on_brightnessSpinBox_valueChanged(int value);

    void on_brightnessThresSpinBox_valueChanged(int value);

    void on_contrastSlider_valueChanged(int value);
    void on_contrastSpinBox_valueChanged(int value);

    void on_primColorMapComboBox_currentIndexChanged(int index);
    void on_secdColorMapComboBox_currentIndexChanged(int index);

    void on_primOpacitySlider_valueChanged(int value);
    void on_primOpacitySpinBox_valueChanged(int value);

    void on_secdOpacitySlider_valueChanged(int value);
    void on_secdOpacitySpinBox_valueChanged(int value);

    void on_fatRadioBtn_toggled(bool checked);
    void on_waterRadioBtn_toggled(bool checked);
    void on_fatFracRadioBtn_toggled(bool checked);
    void on_waterFracRadioBtn_toggled(bool checked);
    void on_fatWaterRadioBtn_toggled(bool checked);
    void on_waterFatRadioBtn_toggled(bool checked);

    void on_resetViewBtn_clicked();

    void on_EATRadioBtn_toggled(bool checked);
    void on_IMATRadioBtn_toggled(bool checked);
    void on_PAATRadioBtn_toggled(bool checked);
    void on_PATRadioBtn_toggled(bool checked);
    void on_SCATRadioBtn_toggled(bool checked);
    void on_VATRadioBtn_toggled(bool checked);

    void on_EATCheckBox_toggled(bool checked);
    void on_IMATCheckBox_toggled(bool checked);
    void on_PAATCheckBox_toggled(bool checked);
    void on_PATCheckBox_toggled(bool checked);
    void on_SCATCheckBox_toggled(bool checked);
    void on_VATCheckBox_toggled(bool checked);

    void on_drawPointsBtn_toggled(bool checked);
    void on_eraserBtn_toggled(bool checked);

    void on_eraserBrushWidthComboBox_currentIndexChanged(int index);

    void undoStack_canUndoChanged(bool canUndo);
    void undoStack_canRedoChanged(bool canRedo);
};

#endif // VIEW_AXIALCORONALHIRES_H
