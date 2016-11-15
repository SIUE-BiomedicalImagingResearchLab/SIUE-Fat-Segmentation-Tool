#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QDesktopWidget>

#include <nifti/include/nifti1.h>
#include <nifti/include/fslio.h>

#include "nifti.hpp"
#include "util.hpp"
#include "exception.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();

    void on_actionOpen_triggered();

private:
    Ui::MainWindow *ui;

    nifti_image *fatUpperImage;
    nifti_image *fatLowerImage;
    nifti_image *waterUpperImage;
    nifti_image *waterLowerImage;

    void readSettings();
    void writeSettings();

    void checkNIFTIImages();
};

#endif // MAINWINDOW_H
