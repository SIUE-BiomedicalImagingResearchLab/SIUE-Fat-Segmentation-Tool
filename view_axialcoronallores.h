#ifndef VIEW_AXIALCORONALLORES_H
#define VIEW_AXIALCORONALLORES_H

#include <QWidget>

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

#include <nifti/nifti1.h>
#include <nifti/nifti1_io.h>

#include "displayinfo.h"
#include "niftimage.h"
#include "util.h"
#include "exception.h"
#include "subjectconfig.h"

namespace Ui {
class viewAxialCoronalLoRes;
}

class MainWindow;

class viewAxialCoronalLoRes : public QWidget
{
    Q_OBJECT

private:
    Ui::viewAxialCoronalLoRes *ui;

    NIFTImage *fatImage;
    NIFTImage *waterImage;

    QUndoView *undoView;
    QUndoStack *undoStack;

    SubjectConfig *subConfig;

    QString saveTracingResultsPath;

    QShortcut *EATShortcut;
    QShortcut *IMATShortcut;
    QShortcut *PAATShortcut;
    QShortcut *PATShortcut;
    QShortcut *SCATShortcut;
    QShortcut *VATShortcut;

    QShortcut *upShortcut;
    QShortcut *downShortcut;
    QShortcut *leftShortcut;
    QShortcut *rightShortcut;

public:
    explicit viewAxialCoronalLoRes(QWidget *parent, NIFTImage *fatImage, NIFTImage *waterImage, SubjectConfig *subConfig);
    ~viewAxialCoronalLoRes();

    MainWindow *parentMain();

    bool loadImage(QString path);
    void enableSettings();
    void setupDefaults();

    void changeSliceView(SliceDisplayType newType);
    void changeTracingLayer(TracingLayer newLayer);

    friend class viewAxialCoronalHiRes;

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

    // Default to true so that the activated() signal in QShortcut can call this
    void on_EATRadioBtn_toggled(bool checked = true);
    void on_IMATRadioBtn_toggled(bool checked = true);
    void on_PAATRadioBtn_toggled(bool checked = true);
    void on_PATRadioBtn_toggled(bool checked = true);
    void on_SCATRadioBtn_toggled(bool checked = true);
    void on_VATRadioBtn_toggled(bool checked = true);

    void on_EATCheckBox_toggled(bool checked);
    void on_IMATCheckBox_toggled(bool checked);
    void on_PAATCheckBox_toggled(bool checked);
    void on_PATCheckBox_toggled(bool checked);
    void on_SCATCheckBox_toggled(bool checked);
    void on_VATCheckBox_toggled(bool checked);

    void undoStack_canUndoChanged(bool canUndo);
    void undoStack_canRedoChanged(bool canRedo);
};

#endif // VIEW_AXIALCORONALLORES_H
