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

#include "view_axialcoronallores.h"
#include "view_axialcoronalhires.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    Ui::MainWindow *ui;

    QString defaultOpenDir;
    QString defaultSaveDir;

    WindowViewType windowViewType;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void readSettings();
    void writeSettings();

    void switchView(WindowViewType type, bool showPrompt = true);

    friend class viewAxialCoronalLoRes;
    friend class viewAxialCoronalHiRes;

private slots:
    void on_actionExit_triggered();

    void on_actionAbout_triggered();

    void on_actionAxialCoronalLoRes_triggered(bool checked);
    void on_actionAxialCoronalHiRes_triggered(bool checked);
};

#endif // MAINWINDOW_H
