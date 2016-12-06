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

#include <nifti/include/nifti1.h>
#include <nifti/include/fslio.h>

#include "niftimage.h"
#include "util.hpp"
#include "exception.hpp"

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

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void readSettings();
    void writeSettings();

private slots:
    void on_actionExit_triggered();

    void on_actionOpen_triggered();

    void on_sliceSlider_valueChanged(int value);
    void on_sliceSpinBox_valueChanged(int value);

    void on_brightnessSlider_valueChanged(int value);
    void on_brightnessSpinBox_valueChanged(int value);

    void on_contrastSlider_valueChanged(int value);
    void on_contrastSpinBox_valueChanged(int value);

    void on_colorMapComboBox_currentIndexChanged(int index);

    void on_fatRadioBtn_toggled(bool checked);

    void on_waterRadioBtn_toggled(bool checked);

    void on_fatFracRadioBtn_toggled(bool checked);

    void on_waterFracRadioBtn_toggled(bool checked);

    void on_resetViewBtn_clicked();

    void undoStack_canUndoChanged(bool canUndo);
    void undoStack_canRedoChanged(bool canRedo);

    void on_actionShow_History_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
};

#endif // MAINWINDOW_H
