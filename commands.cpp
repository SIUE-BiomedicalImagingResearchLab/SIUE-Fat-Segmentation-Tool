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
    this->delta = QVector3D(delta); //QVector3D((delta.x() * 2.0f / widget->width()), (-delta.y() * 2.0f / widget->height()), 0.0f);
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

// CoronalMoveCommand
// --------------------------------------------------------------------------------------------------------------------
CoronalMoveCommand::CoronalMoveCommand(QPointF delta, CoronalSliceWidget *widget, CommandID ID, QUndoCommand *parent) : QUndoCommand(parent)
{
    // Delta is in OpenGL coordinate system already
    this->delta = QVector3D(delta);

    //this->delta = QVector3D(widget->getWindowToOpenGLMatrix() * delta);

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

// LocationChangeCommand
// --------------------------------------------------------------------------------------------------------------------
LocationChangeCommand::LocationChangeCommand(QVector4D newLocation, AxialSliceWidget *axialWidget, CoronalSliceWidget *coronalWidget,
                                             QSlider *axialSlider, QSpinBox *axialSpinBox, QSlider *coronalSlider, QSpinBox *coronalSpinBox,
                                             QSlider *saggitalSlider, QSpinBox *saggitalSpinBox, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->oldLocation = axialWidget->getLocation();
    this->newLocation = axialWidget->transformLocation(newLocation);
    this->axialWidget = axialWidget;
    this->coronalWidget = coronalWidget;

    this->axialSlider = axialSlider;
    this->axialSpinBox = axialSpinBox;

    this->coronalSlider = coronalSlider;
    this->coronalSpinBox = coronalSpinBox;

    this->saggitalSlider = saggitalSlider;
    this->saggitalSpinBox = saggitalSpinBox;

    // Set text shown on QUndoView
    QVector4D delta = newLocation - oldLocation;
    QString str("Move to ");

    if (delta.x())
        str += QObject::tr("saggital slice %1, ").arg(newLocation.x());

    if (delta.y())
        str += QObject::tr("coronal slice %1, ").arg(newLocation.y());

    if (delta.z())
        str += QObject::tr("axial slice %1, ").arg(newLocation.z());

    if (delta.w())
        str += QObject::tr("time slice %1, ").arg(newLocation.w());

    str.remove(str.length() - 2, 2);
    setText(str);
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

    if (delta.z() && axialSlider && axialSpinBox)
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

    if (delta.y() && coronalSlider && coronalSpinBox)
    {
        bool prev = coronalSlider->blockSignals(true);
        coronalSlider->setValue(newLocation.y());
        coronalSlider->blockSignals(prev);

        prev = coronalSpinBox->blockSignals(true);
        coronalSpinBox->setValue(newLocation.y());
        coronalSpinBox->blockSignals(prev);
    }

    if (delta.x() && saggitalSlider && saggitalSpinBox)
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

    // Update text shown on QUndoView
    QVector4D delta = newLocation - oldLocation;
    QString str("Move to ");

    if (delta.x())
        str.append(QObject::tr("saggital slice %1, ").arg(newLocation.x()));

    if (delta.y())
        str.append(QObject::tr("coronal slice %1, ").arg(newLocation.y()));

    if (delta.z())
        str.append(QObject::tr("axial slice %1, ").arg(newLocation.z()));

    if (delta.w())
        str.append(QObject::tr("time slice %1, ").arg(newLocation.w()));

    str.remove(str.length() - 2, 2);
    setText(str);

    //if (newLocation == oldLocation)
    //    setObsolete(true);

    return true;
}

// BrightnessChangeCommand
// --------------------------------------------------------------------------------------------------------------------
BrightnessChangeCommand::BrightnessChangeCommand(float newBrightness, AxialSliceWidget *widget, QSlider *slider, QSpinBox *spinBox, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->oldBrightness = widget->getBrightness();
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

// ContrastChangeCommand
// --------------------------------------------------------------------------------------------------------------------
ContrastChangeCommand::ContrastChangeCommand(float newContrast, AxialSliceWidget *widget, QSlider *slider, QSpinBox *spinBox, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->oldContrast = widget->getBrightness();
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

// PrimColorMapChangeCommand
// --------------------------------------------------------------------------------------------------------------------
PrimColorMapChangeCommand::PrimColorMapChangeCommand(ColorMap newColor, AxialSliceWidget *widget, QComboBox *comboBox, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->oldColor = widget->getPrimColorMap();
    this->newColor = newColor;
    this->widget = widget;
    this->comboBox = comboBox;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Primary color map set to %1").arg(comboBox->itemText((int)newColor)));
}

void PrimColorMapChangeCommand::undo()
{
    // Go back to the old color map
    widget->setPrimColorMap(oldColor);

    // Set the combo box value to be equal to the old color map. It blocks all signals from the setValue
    //so that way the valueChanged is not called again and creates a duplicate ColorMapChangeCommand.
    bool prev = comboBox->blockSignals(true);
    comboBox->setCurrentIndex((int)oldColor);
    comboBox->blockSignals(prev);
}

void PrimColorMapChangeCommand::redo()
{
    // Go to the new color map
    widget->setPrimColorMap(newColor);

    // Set the combo box value to be equal to the new color map. It blocks all signals from the setValue
    // so that way the valueChanged is not called again and creates a duplicate ColorMapChangeCommand.
    bool prev = comboBox->blockSignals(true);
    comboBox->setCurrentIndex((int)newColor);
    comboBox->blockSignals(prev);
}

// SecdColorMapChangeCommand
// --------------------------------------------------------------------------------------------------------------------
SecdColorMapChangeCommand::SecdColorMapChangeCommand(ColorMap newColor, AxialSliceWidget *widget, QComboBox *comboBox, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->oldColor = widget->getSecdColorMap();
    this->newColor = newColor;
    this->widget = widget;
    this->comboBox = comboBox;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Secondary color map set to %1").arg(comboBox->itemText((int)newColor)));
}

void SecdColorMapChangeCommand::undo()
{
    // Go back to the old color map
    widget->setSecdColorMap(oldColor);

    // Set the combo box value to be equal to the old color map. It blocks all signals from the setValue
    //so that way the valueChanged is not called again and creates a duplicate ColorMapChangeCommand.
    bool prev = comboBox->blockSignals(true);
    comboBox->setCurrentIndex((int)oldColor);
    comboBox->blockSignals(prev);
}

void SecdColorMapChangeCommand::redo()
{
    // Go to the new color map
    widget->setSecdColorMap(newColor);

    // Set the combo box value to be equal to the new color map. It blocks all signals from the setValue
    // so that way the valueChanged is not called again and creates a duplicate ColorMapChangeCommand.
    bool prev = comboBox->blockSignals(true);
    comboBox->setCurrentIndex((int)newColor);
    comboBox->blockSignals(prev);
}

// PrimOpacityChangeCommand
// --------------------------------------------------------------------------------------------------------------------
PrimOpacityChangeCommand::PrimOpacityChangeCommand(float newOpacity, AxialSliceWidget *widget, QSlider *slider, QSpinBox *spinBox, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->oldOpacity = widget->getPrimOpacity();
    this->newOpacity = newOpacity;
    this->widget = widget;
    this->slider = slider;
    this->spinBox = spinBox;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Primary opacity set to %1%").arg(int(newOpacity * 100.0f)));
}

void PrimOpacityChangeCommand::undo()
{
    // Go back to the old opacity
    widget->setPrimOpacity(oldOpacity);

    // Set the slider value to be equal to the old contrast. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new PrimOpacityChangeCommand.
    bool prev = slider->blockSignals(true);
    slider->setValue(int(oldOpacity * 100.0f));
    slider->blockSignals(prev);

    // Set the spin box value to be equal to the old contrast. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new PrimOpacityChangeCommand.
    prev = spinBox->blockSignals(true);
    spinBox->setValue(int(oldOpacity * 100.0f));
    spinBox->blockSignals(prev);
}

void PrimOpacityChangeCommand::redo()
{
    // Go to the new brightness
    widget->setPrimOpacity(newOpacity);

    // Set the slider value to be equal to the new contrast. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new PrimOpacityChangeCommand.
    bool prev = slider->blockSignals(true);
    slider->setValue(int(newOpacity * 100.0f));
    slider->blockSignals(prev);

    // Set the spin box value to be equal to the new contrast. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new PrimOpacityChangeCommand.
    prev = spinBox->blockSignals(true);
    spinBox->setValue(int(newOpacity * 100.0f));
    spinBox->blockSignals(prev);
}

bool PrimOpacityChangeCommand::mergeWith(const QUndoCommand *command)
{
    const PrimOpacityChangeCommand *primOpacityChangeCommand = static_cast<const PrimOpacityChangeCommand *>(command);

    // Since the newest command will get merged with the older command, this means the oldOpacity of the current command (this) will stay
    // the same but the newOpacity will change to what the merged command is
    newOpacity = primOpacityChangeCommand->newOpacity;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Primary opacity set to %1%").arg(int(newOpacity * 100.0f)));

    //if (newOpacity == oldOpacity)
    //    setObsolete(true);

    return true;
}

// SecdOpacityChangeCommand
// --------------------------------------------------------------------------------------------------------------------
SecdOpacityChangeCommand::SecdOpacityChangeCommand(float newOpacity, AxialSliceWidget *widget, QSlider *slider, QSpinBox *spinBox, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->oldOpacity = widget->getSecdOpacity();
    this->newOpacity = newOpacity;
    this->widget = widget;
    this->slider = slider;
    this->spinBox = spinBox;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Secondary opacity set to %1%").arg(int(newOpacity * 100.0f)));
}

void SecdOpacityChangeCommand::undo()
{
    // Go back to the old opacity
    widget->setSecdOpacity(oldOpacity);

    // Set the slider value to be equal to the old contrast. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new SecdOpacityChangeCommand.
    bool prev = slider->blockSignals(true);
    slider->setValue(int(oldOpacity * 100.0f));
    slider->blockSignals(prev);

    // Set the spin box value to be equal to the old contrast. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new SecdOpacityChangeCommand.
    prev = spinBox->blockSignals(true);
    spinBox->setValue(int(oldOpacity * 100.0f));
    spinBox->blockSignals(prev);
}

void SecdOpacityChangeCommand::redo()
{
    // Go to the new brightness
    widget->setSecdOpacity(newOpacity);

    // Set the slider value to be equal to the new contrast. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new SecdOpacityChangeCommand.
    bool prev = slider->blockSignals(true);
    slider->setValue(int(newOpacity * 100.0f));
    slider->blockSignals(prev);

    // Set the spin box value to be equal to the new contrast. It blocks all signals from the setValue so that way
    // the valueChanged is not called again and creates a new SecdOpacityChangeCommand.
    prev = spinBox->blockSignals(true);
    spinBox->setValue(int(newOpacity * 100.0f));
    spinBox->blockSignals(prev);
}

bool SecdOpacityChangeCommand::mergeWith(const QUndoCommand *command)
{
    const SecdOpacityChangeCommand *secdOpacityChangeCommand = static_cast<const SecdOpacityChangeCommand *>(command);

    // Since the newest command will get merged with the older command, this means the oldOpacity of the current command (this) will stay
    // the same but the newOpacity will change to what the merged command is
    newOpacity = secdOpacityChangeCommand->newOpacity;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Secondary opacity set to %1%").arg(int(newOpacity * 100.0f)));

    //if (newOpacity == oldOpacity)
    //    setObsolete(true);

    return true;
}

// SliceViewChangeCommand
// --------------------------------------------------------------------------------------------------------------------
SliceViewChangeCommand::SliceViewChangeCommand(SliceDisplayType newDT, AxialSliceWidget *widget, QRadioButton *oldBtn, QRadioButton *newBtn, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->oldDT = widget->getDisplayType();
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

// TracingLayerChangeCommand
// --------------------------------------------------------------------------------------------------------------------
TracingLayerChangeCommand::TracingLayerChangeCommand(TracingLayer newTracingLayer, AxialSliceWidget *widget, QRadioButton *oldBtn, QRadioButton *newBtn, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->oldTracingLayer = widget->getTracingLayer();
    this->newTracingLayer = newTracingLayer;
    this->widget = widget;
    this->oldBtn = oldBtn;
    this->newBtn = newBtn;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Tracing layer set to %1").arg(newBtn->text()));
}

void TracingLayerChangeCommand::undo()
{
    // Go back to the old tracing layer
    widget->setTracingLayer(oldTracingLayer);

    // Check the old radio button which will automatically deselect the currently selected radio button
    // It blocks all signals from being called so that a duplicate TracingLayerChangeCommand is not created
    bool prev = oldBtn->blockSignals(true);
    oldBtn->setChecked(true);
    oldBtn->blockSignals(prev);
}

void TracingLayerChangeCommand::redo()
{
    // Go to the new tracing layer
    widget->setTracingLayer(newTracingLayer);

    // Check the new radio button which will automatically deselect the currently selected radio button
    // It blocks all signals from being called so that a duplicate TracingLayerChangeCommand is not created
    bool prev = newBtn->blockSignals(true);
    newBtn->setChecked(true);
    newBtn->blockSignals(prev);
}

// TracingLayerVisibleChangeCommand
// --------------------------------------------------------------------------------------------------------------------
TracingLayerVisibleChangeCommand::TracingLayerVisibleChangeCommand(TracingLayer tracingLayer, bool newValue, AxialSliceWidget *widget, QCheckBox *btn, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->tracingLayer = tracingLayer;
    this->newValue = newValue;
    this->widget = widget;
    this->btn = btn;

    // Updates text that is shown on QUndoView
    setText(QObject::tr("%1 tracing layer %2").arg(newValue ? "Show" : "Hide").arg(btn->text()));
}

void TracingLayerVisibleChangeCommand::undo()
{
    // Set the tracing layer to the old visibility value
    widget->setTracingLayerVisible(tracingLayer, !newValue);

    // Check/Uncheck the checkbox based on old value (!newValue)
    // It blocks all signals from being called so that a duplicate TracingLayerChangeCommand is not created
    bool prev = btn->blockSignals(true);
    btn->setChecked(!newValue);
    btn->blockSignals(prev);

    widget->update();
}

void TracingLayerVisibleChangeCommand::redo()
{
    // Set the tracing layer to the new visibility value
    widget->setTracingLayerVisible(tracingLayer, newValue);

    // Check/Uncheck the checkbox based on new value (newValue)
    // It blocks all signals from being called so that a duplicate TracingLayerChangeCommand is not created
    bool prev = btn->blockSignals(true);
    btn->setChecked(newValue);
    btn->blockSignals(prev);

    widget->update();
}

// TracingPointsAddCommand
// --------------------------------------------------------------------------------------------------------------------
TracingPointsAddCommand::TracingPointsAddCommand(int index, AxialSliceWidget *widget, QUndoCommand *parent) : QUndoCommand(parent)
{
    this->index = index;
    this->widget = widget;

    // Get a text string based on the layer that points are being added too (current layer)
    QString str;
    switch (widget->getTracingLayer())
    {
        case TracingLayer::EAT: str = "EAT"; break;
        case TracingLayer::IMAT: str = "IMAT"; break;
        case TracingLayer::PAAT: str = "PAAT"; break;
        case TracingLayer::PAT: str = "PAT"; break;
        case TracingLayer::SCAT: str = "SCAT"; break;
        case TracingLayer::VAT: str = "VAT"; break;
    }

    // Updates text that is shown on QUndoView
    setText(QObject::tr("Added points to %1 layer").arg(str));
}

void TracingPointsAddCommand::undo()
{
    // Get a vector of the points contained in the current layer and axial slice
    auto &layerPoints = widget->getLayerPoints();

    // If the number of points is below the index number, then something is wrong
    // because there are no points to copy
    if (layerPoints.size() <= index)
    {
        qDebug() << "Number of points contained in layerPoints is less than index. Layer points size: " << layerPoints.size() << " Index: " << index;
        return;
    }

    // Store points start from index to end of vector. Then erase the points from layerPoints.
    // Note: This is taken as a reference, so this will actually erase the data in AxialSliceWidget points variable
    points.assign(layerPoints.begin() + index, layerPoints.end());
    layerPoints.erase(layerPoints.begin() + index, layerPoints.end());

    // Tell AxialSliceWidget to redraw the scene since points were removed
    widget->update();
}

void TracingPointsAddCommand::redo()
{
    // Get a vector of the points contained in the current layer and axial slice
    auto &layerPoints = widget->getLayerPoints();

    // If there are no points to restore, do nothing. Presumably, this is the first
    // call of redo() when inserting on stack.
    if (points.size() == 0)
        return;

    layerPoints.insert(layerPoints.end(), points.begin(), points.end());
    points.clear();

    // Tell AxialSliceWidget to redraw the scene since points were added
    widget->update();
}

int TracingPointsAddCommand::getIndex()
{
    return index;
}
