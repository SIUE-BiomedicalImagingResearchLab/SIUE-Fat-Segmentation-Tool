#include "commands.h"
#include "axialslicewidget.h"
#include "coronalslicewidget.h"
#include "mainwindow.h"

// AxialMoveCommand
// --------------------------------------------------------------------------------------------------------------------
AxialMoveCommand::AxialMoveCommand(QPointF delta, AxialSliceWidget *widget, CommandID ID, QUndoCommand *parent) : QUndoCommand(parent)
{
    // Divide delta by respective width/height of screen and multiply by 2.0f
    // This is because the OpenGL range is -1.0f -> 1.0f(2.0 total) and the width/height of the screen is given.
    // This converts from window coordinates to OpenGL coordinates
    this->delta = QVector3D((delta.x() * 2.0f / widget->width()), (-delta.y() * 2.0f / widget->height()), 0.0f);
    this->widget = widget;
    this->ID = ID;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Move axial image by (%1%, %2%)").arg(int(delta.x() * 100.0f)).arg(int(delta.y() * 100.0f)));
}

void AxialMoveCommand::undo()
{
    // Divide delta by respective width/height of screen and add it to the translation
    widget->rtranslation() -= delta;

    // Tell the screen to draw itself since the scene changed
    widget->updateCrosshairLine();
    widget->update();
}

void AxialMoveCommand::redo()
{
    // Add delta to translation
    widget->rtranslation() += delta;

    // Tell the screen to draw itself since the scene changed
    widget->updateCrosshairLine();
    widget->update();
}

bool AxialMoveCommand::mergeWith(const QUndoCommand *command)
{
    const AxialMoveCommand *moveCommand = static_cast<const AxialMoveCommand *>(command);

    // Merge the commands by adding their respective deltas
    delta += moveCommand->delta;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Move axial image by (%1%, %2%)").arg(int(delta.x()*100.0f)).arg(int(delta.y()*100.0f)));

    //if (delta.length() == 0)
    //    setObsolete(true);

    return true;
}

AxialMoveCommand::~AxialMoveCommand()
{

}

// CoronalMoveCommand
// --------------------------------------------------------------------------------------------------------------------
CoronalMoveCommand::CoronalMoveCommand(QPointF delta, CoronalSliceWidget *widget, CommandID ID, QUndoCommand *parent) : QUndoCommand(parent)
{
    // Divide delta by respective width/height of screen and multiply by 2.0f
    // This is because the OpenGL range is -1.0f -> 1.0f(2.0 total) and the width/height of the screen is given.
    // This converts from window coordinates to OpenGL coordinates
    this->delta = QVector3D((delta.x() * 2.0f / widget->width()), (-delta.y() * 2.0f / widget->height()), 0.0f);
    this->widget = widget;
    this->ID = ID;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Move coronal image by (%1%, %2%)").arg(int(delta.x() * 100.0f)).arg(int(delta.y() * 100.0f)));
}

void CoronalMoveCommand::undo()
{
    // Divide delta by respective width/height of screen and add it to the translation
    widget->rtranslation() -= delta;

    // Tell the screen to draw itself since the scene changed
    widget->updateCrosshairLine();
    widget->update();
}

void CoronalMoveCommand::redo()
{
    // Add delta to translation
    widget->rtranslation() += delta;

    // Tell the screen to draw itself since the scene changed
    widget->updateCrosshairLine();
    widget->update();
}

bool CoronalMoveCommand::mergeWith(const QUndoCommand *command)
{
    const CoronalMoveCommand *moveCommand = static_cast<const CoronalMoveCommand *>(command);

    // Merge the commands by adding their respective deltas
    delta += moveCommand->delta;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Move coronal image by (%1%, %2%)").arg(int(delta.x()*100.0f)).arg(int(delta.y()*100.0f)));

    //if (delta.length() == 0)
    //    setObsolete(true);

    return true;
}

CoronalMoveCommand::~CoronalMoveCommand()
{

}

// AxialScaleCommand
// --------------------------------------------------------------------------------------------------------------------
AxialScaleCommand::AxialScaleCommand(float scaling, AxialSliceWidget *widget, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->scaling = scaling;
    this->widget = widget;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Scale axial image by %1%").arg(int(scaling * 100.0f)));
}

void AxialScaleCommand::undo()
{
    // Add the scaling to the total scaling to zoom in or out
    widget->rscaling() -= scaling;

    // Clamp it between 0.05f (5%) to 3.0f (300%)
    widget->rscaling() = std::max(std::min(widget->rscaling(), 3.0f), 0.05f);

    // Tell the screen to draw itself since the scene changed
    widget->updateCrosshairLine();
    widget->update();
}

void AxialScaleCommand::redo()
{
    // Add the scaling to the total scaling to zoom in or out
    widget->rscaling() += scaling;

    // Clamp it between 0.05f (5%) to 3.0f (300%)
    widget->rscaling() = std::max(std::min(widget->rscaling(), 3.0f), 0.05f);

    // Tell the screen to draw itself since the scene changed
    widget->updateCrosshairLine();
    widget->update();
}

bool AxialScaleCommand::mergeWith(const QUndoCommand *command)
{
    const AxialScaleCommand *scaleCommand = static_cast<const AxialScaleCommand *>(command);

    // Merge the two commands by adding the scaling component
    scaling += scaleCommand->scaling;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Scale axial image by %1%").arg(int(scaling*100.0f)));

    // When building this executable, the setObsolete command requires a special patch in Qt to work
    // correctly. It is something that I edited in the Qt source code and contributed to the Qt source
    // code. It is likely that it will not become apart of the built libraries until Qt 5.9+. Therefore,
    // if you do not want to bother building the Qt libraries(6+ hours of time at least), then comment out
    // all instances of setObsolete and it should work. However, if you are interested, download the source
    // code and cherry-pick the patch at: https://codereview.qt-project.org/#/c/178852/
    //if (scaling == 0.0f)
    //    setObsolete(true);

    return true;
}

AxialScaleCommand::~AxialScaleCommand()
{

}

// CoronalScaleCommand
// --------------------------------------------------------------------------------------------------------------------
CoronalScaleCommand::CoronalScaleCommand(float scaling, CoronalSliceWidget *widget, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->scaling = scaling;
    this->widget = widget;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Scale coronal image by %1%").arg(int(scaling * 100.0f)));
}

void CoronalScaleCommand::undo()
{
    // Add the scaling to the total scaling to zoom in or out
    widget->rscaling() -= scaling;

    // Clamp it between 0.05f (5%) to 3.0f (300%)
    widget->rscaling() = std::max(std::min(widget->rscaling(), 3.0f), 0.05f);

    // Tell the screen to draw itself since the scene changed
    widget->updateCrosshairLine();
    widget->update();
}

void CoronalScaleCommand::redo()
{
    // Add the scaling to the total scaling to zoom in or out
    widget->rscaling() += scaling;

    // Clamp it between 0.05f (5%) to 3.0f (300%)
    widget->rscaling() = std::max(std::min(widget->rscaling(), 3.0f), 0.05f);

    // Tell the screen to draw itself since the scene changed
    widget->updateCrosshairLine();
    widget->update();
}

bool CoronalScaleCommand::mergeWith(const QUndoCommand *command)
{
    const CoronalScaleCommand *scaleCommand = static_cast<const CoronalScaleCommand *>(command);

    // Merge the two commands by adding the scaling component
    scaling += scaleCommand->scaling;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Scale coronal image by %1%").arg(int(scaling*100.0f)));

    // When building this executable, the setObsolete command requires a special patch in Qt to work
    // correctly. It is something that I edited in the Qt source code and contributed to the Qt source
    // code. It is likely that it will not become apart of the built libraries until Qt 5.9+. Therefore,
    // if you do not want to bother building the Qt libraries(6+ hours of time at least), then comment out
    // all instances of setObsolete and it should work. However, if you are interested, download the source
    // code and cherry-pick the patch at: https://codereview.qt-project.org/#/c/178852/
    //if (scaling == 0.0f)
    //    setObsolete(true);

    return true;
}

CoronalScaleCommand::~CoronalScaleCommand()
{

}

// LocationChangeCommand
// --------------------------------------------------------------------------------------------------------------------
LocationChangeCommand::LocationChangeCommand(QVector4D oldLocation, QVector4D newLocation, AxialSliceWidget *axialWidget, CoronalSliceWidget *coronalWidget,
                                             QSlider *axialSlider, QSpinBox *axialSpinBox, QSlider *coronalSlider, QSpinBox *coronalSpinBox,
                                             QSlider *saggitalSlider, QSpinBox *saggitalSpinBox, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->oldLocation = oldLocation;
    this->newLocation = newLocation;
    this->axialWidget = axialWidget;
    this->coronalWidget = coronalWidget;

    this->axialSlider = axialSlider;
    this->axialSpinBox = axialSpinBox;

    this->coronalSlider = coronalSlider;
    this->coronalSpinBox = coronalSpinBox;

    this->saggitalSlider = saggitalSlider;
    this->saggitalSpinBox = saggitalSpinBox;

    // Updates text that is shown on QUndoView
    //setText(QObject::tr("Move to slice %1").arg(newSlice));
}

void LocationChangeCommand::undo()
{
    // Go back to the old slice
    axialWidget->setLocation(oldLocation);
    coronalWidget->setLocation(oldLocation);

    QVector4D delta = newLocation - oldLocation;

    if (delta.z() && axialSlider && axialSpinBox)
    {
        // Set the slider value to be equal to the new slice. It blocks all signals from the setValue so that way
        // the valueChanged is not called again and creates a new LocationChangeCommand.
        bool prev = axialSlider->blockSignals(true);
        axialSlider->setValue(oldLocation.z());
        axialSlider->blockSignals(prev);

        // Set the spin box value to be equal to the new slice. It blocks all signals from the setValue so that way
        // the valueChanged is not called again and creates a new LocationChangeCommand.
        prev = axialSpinBox->blockSignals(true);
        axialSpinBox->setValue(oldLocation.z());
        axialSpinBox->blockSignals(prev);
    }

    if (delta.y() && coronalSlider && coronalSpinBox)
    {
        bool prev = coronalSlider->blockSignals(true);
        coronalSlider->setValue(oldLocation.y());
        coronalSlider->blockSignals(prev);

        prev = coronalSpinBox->blockSignals(true);
        coronalSpinBox->setValue(oldLocation.y());
        coronalSpinBox->blockSignals(prev);
    }

    if (delta.x() && saggitalSlider && saggitalSpinBox)
    {
        bool prev = saggitalSlider->blockSignals(true);
        saggitalSlider->setValue(oldLocation.x());
        saggitalSlider->blockSignals(prev);

        prev = saggitalSpinBox->blockSignals(true);
        saggitalSpinBox->setValue(oldLocation.x());
        saggitalSpinBox->blockSignals(prev);
    }
}

void LocationChangeCommand::redo()
{
    // Go to new location
    axialWidget->setLocation(newLocation);
    coronalWidget->setLocation(newLocation);

    QVector4D delta = newLocation - oldLocation;

    if (delta.z() != Location::NoChange && delta.z() && axialSlider && axialSpinBox)
    {
        // Set the slider value to be equal to the new slice. It blocks all signals from the setValue so that way
        // the valueChanged is not called again and creates a new LocationChangeCommand.
        bool prev = axialSlider->blockSignals(true);
        axialSlider->setValue(newLocation.z());
        axialSlider->blockSignals(prev);

        // Set the spin box value to be equal to the new slice. It blocks all signals from the setValue so that way
        // the valueChanged is not called again and creates a new LocationChangeCommand.
        prev = axialSpinBox->blockSignals(true);
        axialSpinBox->setValue(newLocation.z());
        axialSpinBox->blockSignals(prev);
    }

    if (delta.y() != Location::NoChange && delta.y() && coronalSlider && coronalSpinBox)
    {
        bool prev = coronalSlider->blockSignals(true);
        coronalSlider->setValue(newLocation.y());
        coronalSlider->blockSignals(prev);

        prev = coronalSpinBox->blockSignals(true);
        coronalSpinBox->setValue(newLocation.y());
        coronalSpinBox->blockSignals(prev);
    }

    if (delta.x() != Location::NoChange && delta.x() && saggitalSlider && saggitalSpinBox)
    {
        bool prev = saggitalSlider->blockSignals(true);
        saggitalSlider->setValue(newLocation.x());
        saggitalSlider->blockSignals(prev);

        prev = saggitalSpinBox->blockSignals(true);
        saggitalSpinBox->setValue(newLocation.x());
        saggitalSpinBox->blockSignals(prev);
    }
}

bool LocationChangeCommand::mergeWith(const QUndoCommand *command)
{
    const LocationChangeCommand *sliceChangeCommand = static_cast<const LocationChangeCommand *>(command);

    // Since the newest command will get merged with the older command, this means the oldLocation of the current command (this) will stay
    // the same but the newLocation will change to what the merged command is
    newLocation = sliceChangeCommand->newLocation;

    // Updates text that is shown on QUndoView
    //setText(QObject::tr("Move to slice %1").arg(newSlice));

    //if (newLocation == oldLocation)
    //    setObsolete(true);

    return true;
}

LocationChangeCommand::~LocationChangeCommand()
{

}

// BrightnessChangeCommand
// --------------------------------------------------------------------------------------------------------------------
BrightnessChangeCommand::BrightnessChangeCommand(float oldBrightness, float newBrightness, AxialSliceWidget *widget, QSlider *slider, QSpinBox *spinBox, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->oldBrightness = oldBrightness;
    this->newBrightness = newBrightness;
    this->widget = widget;
    this->slider = slider;
    this->spinBox = spinBox;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Brightness set to %1%").arg(int(newBrightness * 100.0f)));
}

void BrightnessChangeCommand::undo()
{
    // Go back to the old brightness
    widget->setBrightness(oldBrightness);

    // Set the slider value to be equal to the old brightness. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new BrightnessChangeCommand.
    bool prev = slider->blockSignals(true);
    slider->setValue(int(oldBrightness * 100.0f));
    slider->blockSignals(prev);

    // Set the spin box value to be equal to the old brightness. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new BrightnessChangeCommand.
    prev = spinBox->blockSignals(true);
    spinBox->setValue(int(oldBrightness * 100.0f));
    spinBox->blockSignals(prev);
}

void BrightnessChangeCommand::redo()
{
    // Go to the new brightness
    widget->setBrightness(newBrightness);

    // Set the slider value to be equal to the new brightness. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new BrightnessChangeCommand.
    bool prev = slider->blockSignals(true);
    slider->setValue(int(newBrightness * 100.0f));
    slider->blockSignals(prev);

    // Set the spin box value to be equal to the new brightness. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new BrightnessChangeCommand.
    prev = spinBox->blockSignals(true);
    spinBox->setValue(int(newBrightness * 100.0f));
    spinBox->blockSignals(prev);
}

bool BrightnessChangeCommand::mergeWith(const QUndoCommand *command)
{
    const BrightnessChangeCommand *brightnessChangeCommand = static_cast<const BrightnessChangeCommand *>(command);

    // Since the newest command will get merged with the older command, this means the oldBrightness of the current command (this) will stay
    // the same but the newBrightness will change to what the merged command is
    newBrightness = brightnessChangeCommand->newBrightness;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Brightness set to %1%").arg(int(newBrightness * 100.0f)));

    //if (newBrightness == oldBrightness)
    //   setObsolete(true);

    return true;
}

BrightnessChangeCommand::~BrightnessChangeCommand()
{

}

// ContrastChangeCommand
// --------------------------------------------------------------------------------------------------------------------
ContrastChangeCommand::ContrastChangeCommand(float oldContrast, float newContrast, AxialSliceWidget *widget, QSlider *slider, QSpinBox *spinBox, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->oldContrast = oldContrast;
    this->newContrast = newContrast;
    this->widget = widget;
    this->slider = slider;
    this->spinBox = spinBox;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Contrast set to %1%").arg(int(newContrast * 100.0f)));
}

void ContrastChangeCommand::undo()
{
    // Go back to the old contrast
    widget->setContrast(oldContrast);

    // Set the slider value to be equal to the old contrast. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new ContrastChangeCommand.
    bool prev = slider->blockSignals(true);
    slider->setValue(int(oldContrast * 100.0f));
    slider->blockSignals(prev);

    // Set the spin box value to be equal to the old contrast. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new ContrastChangeCommand.
    prev = spinBox->blockSignals(true);
    spinBox->setValue(int(oldContrast * 100.0f));
    spinBox->blockSignals(prev);
}

void ContrastChangeCommand::redo()
{
    // Go to the new brightness
    widget->setContrast(newContrast);

    // Set the slider value to be equal to the new contrast. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new ContrastChangeCommand.
    bool prev = slider->blockSignals(true);
    slider->setValue(int(newContrast * 100.0f));
    slider->blockSignals(prev);

    // Set the spin box value to be equal to the new contrast. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new ContrastChangeCommand.
    prev = spinBox->blockSignals(true);
    spinBox->setValue(int(newContrast * 100.0f));
    spinBox->blockSignals(prev);
}

bool ContrastChangeCommand::mergeWith(const QUndoCommand *command)
{
    const ContrastChangeCommand *contrastChangeCommand = static_cast<const ContrastChangeCommand *>(command);

    // Since the newest command will get merged with the older command, this means the oldContrast of the current command (this) will stay
    // the same but the newContrast will change to what the merged command is
    newContrast = contrastChangeCommand->newContrast;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Contrast set to %1%").arg(int(newContrast * 100.0f)));

    //if (newContrast == oldContrast)
    //    setObsolete(true);

    return true;
}

ContrastChangeCommand::~ContrastChangeCommand()
{

}

// ColorMapChangeCommand
// --------------------------------------------------------------------------------------------------------------------
ColorMapChangeCommand::ColorMapChangeCommand(ColorMap oldColor, ColorMap newColor, AxialSliceWidget *widget, QComboBox *comboBox, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->oldColor = oldColor;
    this->newColor = newColor;
    this->widget = widget;
    this->comboBox = comboBox;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Color map set to %1").arg(comboBox->itemText((int)newColor)));
}

void ColorMapChangeCommand::undo()
{
    // Go back to the old color map
    widget->setColorMap(oldColor);

    // Set the combo box value to be equal to the old color map. It blocks all signals from the setValue
    //so that way the valueChanged is not called again and creates a duplicate ColorMapChangeCommand.
    bool prev = comboBox->blockSignals(true);
    comboBox->setCurrentIndex((int)oldColor);
    comboBox->blockSignals(prev);
}

void ColorMapChangeCommand::redo()
{
    // Go to the new color map
    widget->setColorMap(newColor);

    // Set the combo box value to be equal to the new color map. It blocks all signals from the setValue
    // so that way the valueChanged is not called again and creates a duplicate ColorMapChangeCommand.
    bool prev = comboBox->blockSignals(true);
    comboBox->setCurrentIndex((int)newColor);
    comboBox->blockSignals(prev);
}

ColorMapChangeCommand::~ColorMapChangeCommand()
{

}

// SliceViewChangeCommand
// --------------------------------------------------------------------------------------------------------------------
SliceViewChangeCommand::SliceViewChangeCommand(SliceDisplayType oldDT, SliceDisplayType newDT, AxialSliceWidget *widget, QRadioButton *oldBtn, QRadioButton *newBtn, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->oldDT = oldDT;
    this->newDT = newDT;
    this->widget = widget;
    this->oldBtn = oldBtn;
    this->newBtn = newBtn;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Slice view set to %1").arg(newBtn->text()));
}

void SliceViewChangeCommand::undo()
{
    // Go back to the old slice view
    widget->setDisplayType(oldDT);

    // Check the old radio button which will automatically deselect the currently selected radio button
    // It blocks all signals from being called so that a duplicate SliceViewChangeCommand is not created
    bool prev = oldBtn->blockSignals(true);
    oldBtn->setChecked(true);
    oldBtn->blockSignals(prev);
}

void SliceViewChangeCommand::redo()
{
    // Go to the new slice view
    widget->setDisplayType(newDT);

    // Check the new radio button which will automatically deselect the currently selected radio button
    // It blocks all signals from being called so that a duplicate SliceViewChangeCommand is not created
    bool prev = newBtn->blockSignals(true);
    newBtn->setChecked(true);
    newBtn->blockSignals(prev);
}

SliceViewChangeCommand::~SliceViewChangeCommand()
{

}
