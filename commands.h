#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include <QPointF>
#include <QVector3D>
#include <QVector4D>
#include <QSlider>
#include <QSpinBox>
#include <QComboBox>
#include <QRadioButton>

#include "displayinfo.h"

// Since AxialSliceWidget and MainWindow include this file in its header, it would be an infinite loop if this file included them in their header
// To get around this, the classes are simply declared here and the headers for these classes are included in the source file
class AxialSliceWidget;
class CoronalSliceWidget;
class MainWindow;

// These command IDs are used for merging commands. Only commands with the same ID will be merged.
enum CommandID : int
{
    AxialMove = 1,
    AxialMoveEnd = 10,
    CoronalMove = 11,
    CoronalMoveEnd = 20,
    AxialScale,
    CoronalScale,
    LocationChange,
    BrightnessChange,
    ContrastChange,
    PrimOpacityChange,
    SecdOpacityChange
};

class AxialMoveCommand : public QUndoCommand
{
private:
    QVector3D delta;
    AxialSliceWidget *widget;
    CommandID ID;

public:
    AxialMoveCommand(QPointF delta, AxialSliceWidget *widget, CommandID ID, QUndoCommand *parent = NULL);
    ~AxialMoveCommand();

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return ID; }
};

class CoronalMoveCommand : public QUndoCommand
{
private:
    QVector3D delta;
    CoronalSliceWidget *widget;
    CommandID ID;

public:
    CoronalMoveCommand(QPointF delta, CoronalSliceWidget *widget, CommandID ID, QUndoCommand *parent = NULL);
    ~CoronalMoveCommand();

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return ID; }
};

class AxialScaleCommand : public QUndoCommand
{
private:
    float scaling;
    AxialSliceWidget *widget;

public:
    AxialScaleCommand(float scaling, AxialSliceWidget *widget, QUndoCommand *parent = NULL);
    ~AxialScaleCommand();

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return CommandID::AxialScale; }
};

class CoronalScaleCommand : public QUndoCommand
{
private:
    float scaling;
    CoronalSliceWidget *widget;

public:
    CoronalScaleCommand(float scaling, CoronalSliceWidget *widget, QUndoCommand *parent = NULL);
    ~CoronalScaleCommand();

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return CommandID::CoronalScale; }
};

class LocationChangeCommand : public QUndoCommand
{
private:
    QVector4D oldLocation;
    QVector4D newLocation;
    MainWindow *window;
    AxialSliceWidget *axialWidget;
    CoronalSliceWidget *coronalWidget;

    QSlider *axialSlider;
    QSpinBox *axialSpinBox;

    QSlider *coronalSlider;
    QSpinBox *coronalSpinBox;

    QSlider *saggitalSlider;
    QSpinBox *saggitalSpinBox;

public:
    LocationChangeCommand(QVector4D oldLocation, QVector4D newLocation, AxialSliceWidget *axialWidget, CoronalSliceWidget *coronalWidget,
                          QSlider *axialSlider, QSpinBox *axialSpinBox, QSlider *coronalSlider, QSpinBox *coronalSpinBox,
                          QSlider *saggitalSlider, QSpinBox *saggitalSpinBox, QUndoCommand *parent = NULL);
    ~LocationChangeCommand();

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return CommandID::LocationChange; }
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

class PrimColorMapChangeCommand : public QUndoCommand
{
private:
    ColorMap oldColor;
    ColorMap newColor;
    AxialSliceWidget *widget;
    QComboBox *comboBox;

public:
    PrimColorMapChangeCommand(ColorMap oldColor, ColorMap newColor, AxialSliceWidget *widget, QComboBox *comboBox, QUndoCommand *parent = NULL);
    ~PrimColorMapChangeCommand();

    void undo() override;
    void redo() override;
};

class SecdColorMapChangeCommand : public QUndoCommand
{
private:
    ColorMap oldColor;
    ColorMap newColor;
    AxialSliceWidget *widget;
    QComboBox *comboBox;

public:
    SecdColorMapChangeCommand(ColorMap oldColor, ColorMap newColor, AxialSliceWidget *widget, QComboBox *comboBox, QUndoCommand *parent = NULL);
    ~SecdColorMapChangeCommand();

    void undo() override;
    void redo() override;
};

class PrimOpacityChangeCommand : public QUndoCommand
{
private:
    float oldOpacity;
    float newOpacity;
    AxialSliceWidget *widget;
    QSlider *slider;
    QSpinBox *spinBox;

public:
    PrimOpacityChangeCommand(float oldOpacity, float newOpacity, AxialSliceWidget *widget, QSlider *slider, QSpinBox *spinBox, QUndoCommand *parent = NULL);
    ~PrimOpacityChangeCommand();

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return CommandID::PrimOpacityChange; }
};

class SecdOpacityChangeCommand : public QUndoCommand
{
private:
    float oldOpacity;
    float newOpacity;
    AxialSliceWidget *widget;
    QSlider *slider;
    QSpinBox *spinBox;

public:
    SecdOpacityChangeCommand(float oldOpacity, float newOpacity, AxialSliceWidget *widget, QSlider *slider, QSpinBox *spinBox, QUndoCommand *parent = NULL);
    ~SecdOpacityChangeCommand();

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return CommandID::SecdOpacityChange; }
};

class SliceViewChangeCommand : public QUndoCommand
{
private:
    SliceDisplayType oldDT;
    SliceDisplayType newDT;
    AxialSliceWidget *widget;
    QRadioButton *oldBtn;
    QRadioButton *newBtn;

public:
    SliceViewChangeCommand(SliceDisplayType oldDT, SliceDisplayType newDT, AxialSliceWidget *widget, QRadioButton *oldBtn, QRadioButton *newBtn, QUndoCommand *parent = NULL);
    ~SliceViewChangeCommand();

    void undo() override;
    void redo() override;
};


#endif // COMMANDS_H
