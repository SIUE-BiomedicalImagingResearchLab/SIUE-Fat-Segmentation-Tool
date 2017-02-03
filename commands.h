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
#include <QPushButton>
#include <QCheckBox>

#include "displayinfo.h"

// Since AxialSliceWidget and MainWindow include this file in its header, it would be an infinite loop if this file included them in their header
// To get around this, the classes are simply declared here and the headers for these classes are included in the source file
class AxialSliceWidget;
class CoronalSliceWidget;
class MainWindow;

// These command IDs are used for merging commands. Only commands with the same ID will be merged.
enum class CommandID : int
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

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return (int)ID; }
};

class CoronalMoveCommand : public QUndoCommand
{
private:
    QVector3D delta;
    CoronalSliceWidget *widget;
    CommandID ID;

public:
    CoronalMoveCommand(QPointF delta, CoronalSliceWidget *widget, CommandID ID, QUndoCommand *parent = NULL);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return (int)ID; }
};

class AxialScaleCommand : public QUndoCommand
{
private:
    float scaling;
    AxialSliceWidget *widget;

public:
    AxialScaleCommand(float scaling, AxialSliceWidget *widget, QUndoCommand *parent = NULL);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return (int)CommandID::AxialScale; }
};

class CoronalScaleCommand : public QUndoCommand
{
private:
    float scaling;
    CoronalSliceWidget *widget;

public:
    CoronalScaleCommand(float scaling, CoronalSliceWidget *widget, QUndoCommand *parent = NULL);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return (int)CommandID::CoronalScale; }
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
    LocationChangeCommand(QVector4D newLocation, AxialSliceWidget *axialWidget, CoronalSliceWidget *coronalWidget,
                          QSlider *axialSlider, QSpinBox *axialSpinBox, QSlider *coronalSlider, QSpinBox *coronalSpinBox,
                          QSlider *saggitalSlider, QSpinBox *saggitalSpinBox, QUndoCommand *parent = NULL);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return (int)CommandID::LocationChange; }
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
    BrightnessChangeCommand(float newBrightness, AxialSliceWidget *widget, QSlider *slider, QSpinBox *spinBox, QUndoCommand *parent = NULL);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return (int)CommandID::BrightnessChange; }
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
    ContrastChangeCommand(float newContrast, AxialSliceWidget *widget, QSlider *slider, QSpinBox *spinBox, QUndoCommand *parent = NULL);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return (int)CommandID::ContrastChange; }
};

class PrimColorMapChangeCommand : public QUndoCommand
{
private:
    ColorMap oldColor;
    ColorMap newColor;
    AxialSliceWidget *widget;
    QComboBox *comboBox;

public:
    PrimColorMapChangeCommand(ColorMap newColor, AxialSliceWidget *widget, QComboBox *comboBox, QUndoCommand *parent = NULL);

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
    SecdColorMapChangeCommand(ColorMap newColor, AxialSliceWidget *widget, QComboBox *comboBox, QUndoCommand *parent = NULL);

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
    PrimOpacityChangeCommand(float newOpacity, AxialSliceWidget *widget, QSlider *slider, QSpinBox *spinBox, QUndoCommand *parent = NULL);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return (int)CommandID::PrimOpacityChange; }
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
    SecdOpacityChangeCommand(float newOpacity, AxialSliceWidget *widget, QSlider *slider, QSpinBox *spinBox, QUndoCommand *parent = NULL);

    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override { return (int)CommandID::SecdOpacityChange; }
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
    SliceViewChangeCommand(SliceDisplayType newDT, AxialSliceWidget *widget, QRadioButton *oldBtn, QRadioButton *newBtn, QUndoCommand *parent = NULL);

    void undo() override;
    void redo() override;
};

class TracingLayerChangeCommand : public QUndoCommand
{
private:
    TracingLayer oldTracingLayer;
    TracingLayer newTracingLayer;
    AxialSliceWidget *widget;
    QRadioButton *oldBtn;
    QRadioButton *newBtn;

public:
    TracingLayerChangeCommand(TracingLayer newTracingLayer, AxialSliceWidget *widget, QRadioButton *oldBtn, QRadioButton *newBtn, QUndoCommand *parent = NULL);

    void undo() override;
    void redo() override;
};

class TracingLayerVisibleChangeCommand : public QUndoCommand
{
private:
    TracingLayer tracingLayer;
    bool newValue;
    AxialSliceWidget *widget;
    QCheckBox *btn;

public:
    TracingLayerVisibleChangeCommand(TracingLayer tracingLayer, bool newValue, AxialSliceWidget *widget, QCheckBox *btn, QUndoCommand *parent = NULL);

    void undo() override;
    void redo() override;
};

/* Note: This class will assume that the layer and axial slice that is being drawn on is the current layer and axial slice.
 * This is a safe assumption so long as QUndoStack keeps track of all layer changes and axial slice changes. This will cause
 * a new class to be made for each axial slice AND layer. Also, the user must release their mouse before they can change the
 * slice or layer. (Which will cause the command to be done)
 */
class TracingPointsAddCommand : public QUndoCommand
{
private:
    std::vector<QPoint> points;
    AxialSliceWidget *widget;

public:
    TracingPointsAddCommand(QPoint point, AxialSliceWidget *widget, QUndoCommand *parent = NULL);

    void undo() override;
    void redo() override;

    void addPoint(QPoint newPoint);
};

class DrawModeChangeCommand : public QUndoCommand
{
private:
    DrawMode oldDrawMode;
    DrawMode newDrawMode;
    AxialSliceWidget *widget;
    QPushButton *oldBtn;
    QPushButton *newBtn;

public:
    DrawModeChangeCommand(DrawMode newDrawMode, AxialSliceWidget *widget, QPushButton *oldBtn, QPushButton *newBtn, QString str, QUndoCommand *parent = NULL);

    void undo() override;
    void redo() override;
};

#endif // COMMANDS_H
