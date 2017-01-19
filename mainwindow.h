#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
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
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;

    NIFTImage *fatImage;
    NIFTImage *waterImage;

    QUndoView *undoView;
    QUndoStack *undoStack;

    QString defaultOpenDir;
    QString defaultSaveDir;

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
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void readSettings();
    void writeSettings();

    bool loadImage(QString path);
    void enableSettings();
    void setupDefaults();

    void changeSliceView(SliceDisplayType newType);
    void changeTracingLayer(TracingLayer newLayer);

private slots:
    void on_actionOpen_triggered();

    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();

    void on_actionImportTracingData_triggered();

    void on_actionExit_triggered();

    void on_actionAbout_triggered();

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

    // Default to true so that the activated() signal in QShortcut
    // can call this
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

    void on_actionShow_History_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
};

#endif // MAINWINDOW_H
