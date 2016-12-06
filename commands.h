#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include <QPointF>
#include <QVector3D>
#include <QSlider>
#include <QSpinBox>
#include <QComboBox>
#include <QRadioButton>

// Since AxialSlicWidget and MainWindow include this file in its header, it would be an infinite loop if this file included them in their header
// To get around this, the classes are simply declared here and the headers for these classes are included in the source file
class AxialSliceWidget;
enum ColorMap;
enum AxialDisplayType;

// These command IDs are used for merging commands. Only commands with the same ID will be merged.
enum CommandID
{
    Move = 1,
    MoveEnd = 10,
    Scale,
    SliceChange,
    BrightnessChange,
    ContrastChange
};

class MoveCommand : public QUndoCommand
{
private:
    QVector3D delta;
    AxialSliceWidget *widget;
    CommandID ID;

public:
    MoveCommand(QPointF delta, AxialSliceWidget *widget, CommandID ID, QUndoCommand *parent = NULL);
    ~MoveCommand();

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return ID; }
};

class ScaleCommand : public QUndoCommand
{
private:
    float scaling;
    AxialSliceWidget *widget;

public:
    ScaleCommand(float scaling, AxialSliceWidget *widget, QUndoCommand *parent = NULL);
    ~ScaleCommand();

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return CommandID::Scale; }
};

class SliceChangeCommand : public QUndoCommand
{
private:
    int oldSlice;
    int newSlice;
    AxialSliceWidget *widget;
    QSlider *slider;
    QSpinBox *spinBox;

public:
    SliceChangeCommand(int oldSlice, int newSlice, AxialSliceWidget *widget, QSlider *slider, QSpinBox *spinBox, QUndoCommand *parent = NULL);
    ~SliceChangeCommand();

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return CommandID::SliceChange; }
};

class BrightnessChangeCommand : public QUndoCommand
{
private:
    float oldBrightness;
    float newBrightness;
    AxialSliceWidget *widget;
    QSlider *slider;
    QSpinBox *spinBox;

public:
    BrightnessChangeCommand(float oldBrightness, float newBrightness, AxialSliceWidget *widget, QSlider *slider, QSpinBox *spinBox, QUndoCommand *parent = NULL);
    ~BrightnessChangeCommand();

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return CommandID::BrightnessChange; }
};

class ContrastChangeCommand : public QUndoCommand
{
private:
    float oldContrast;
    float newContrast;
    AxialSliceWidget *widget;
    QSlider *slider;
    QSpinBox *spinBox;

public:
    ContrastChangeCommand(float oldContrast, float newContrast, AxialSliceWidget *widget, QSlider *slider, QSpinBox *spinBox, QUndoCommand *parent = NULL);
    ~ContrastChangeCommand();

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return CommandID::ContrastChange; }
};

class ColorMapChangeCommand : public QUndoCommand
{
private:
    ColorMap oldColor;
    ColorMap newColor;
    AxialSliceWidget *widget;
    QComboBox *comboBox;

public:
    ColorMapChangeCommand(ColorMap oldColor, ColorMap newColor, AxialSliceWidget *widget, QComboBox *comboBox, QUndoCommand *parent = NULL);
    ~ColorMapChangeCommand();

    void undo() override;
    void redo() override;
};

class SliceViewChangeCommand : public QUndoCommand
{
private:
    AxialDisplayType oldDT;
    AxialDisplayType newDT;
    AxialSliceWidget *widget;
    QRadioButton *oldBtn;
    QRadioButton *newBtn;

public:
    SliceViewChangeCommand(AxialDisplayType oldDT, AxialDisplayType newDT, AxialSliceWidget *widget, QRadioButton *oldBtn, QRadioButton *newBtn, QUndoCommand *parent = NULL);
    ~SliceViewChangeCommand();

    void undo() override;
    void redo() override;
};


#endif // COMMANDS_H
