#include "coronalslicewidget.h"
#include "commands.h"

CoronalSliceWidget::CoronalSliceWidget(QWidget *parent) : QOpenGLWidget(parent),
    displayType(SliceDisplayType::FatOnly), fatImage(NULL), waterImage(NULL),
    sliceTexture(NULL), location(0, 0, 0, 0), startPan(false), moveID(CommandID::CoronalMove)
{

}

void CoronalSliceWidget::setup(NIFTImage *fat, NIFTImage *water)
{
    if (!fat || !water)
        return;

    fatImage = fat;
    waterImage = water;

    location = QVector4D(0, 0, 0, 0);
}

bool CoronalSliceWidget::isLoaded() const
{
    return (fatImage->isLoaded() && waterImage->isLoaded());
}

void CoronalSliceWidget::imageLoaded()
{
    sliceTextureInit = false;

    dirty |= Dirty::Slice | Dirty::Crosshair;
    update();
}

void CoronalSliceWidget::setLocation(QVector4D location)
{
    // If there is no fat or water image currently loaded then return with doing nothing.
    if (!isLoaded())
        return;

    QVector4D transformedLocation = transformLocation(location);
    QVector4D delta = transformedLocation - this->location;

    this->location = transformedLocation;

    // If Y value changed, then update the texture
    if (delta.y())
        dirty |= Dirty::Slice;

    // If Z value changed, then update the crosshair line
    if (delta.z())
        dirty |= Dirty::Crosshair;

    update();
}

QVector4D CoronalSliceWidget::getLocation() const
{
    return location;
}

QVector4D CoronalSliceWidget::transformLocation(QVector4D location) const
{
    // This function returns a QVector4D that replaces Location::NoChange's from location variable with actual location value

    QVector4D temp(location.x() == Location::NoChange, location.y() == Location::NoChange, location.z() == Location::NoChange, location.w() == Location::NoChange);
    QVector4D notTemp = QVector4D(1.0f, 1.0f, 1.0f, 1.0f) - temp;
    return (location * notTemp) + (this->location * temp);
}

SliceDisplayType CoronalSliceWidget::getDisplayType() const
{
    return displayType;
}

void CoronalSliceWidget::setDisplayType(SliceDisplayType type)
{
    // If the display type is out of the acceptable range, then do nothing
    if (type < SliceDisplayType::FatOnly || type > SliceDisplayType::WaterFraction)
    {
        qWarning() << "Invalid display type was specified for CoronalSliceWidget: " << (int)type;
        return;
    }

    displayType = type;

    // This will recreate the texture because the display type has changed
    dirty |= Dirty::Slice;
    update();
}

float &CoronalSliceWidget::rscaling()
{
    return scaling;
}

QVector3D &CoronalSliceWidget::rtranslation()
{
    return translation;
}

QMatrix4x4 CoronalSliceWidget::getMVPMatrix() const
{
    // Calculate the ModelViewProjection (MVP) matrix to transform the location of the axial slices
    QMatrix4x4 modelMatrix;

    // Translate the modelMatrix according to translation vector
    // Then scale according to scaling factor
    modelMatrix.translate(translation);
    modelMatrix.scale(scaling);

    // Create the MVP matrix by M * V * P
    return (projectionMatrix * viewMatrix * modelMatrix);
}

QMatrix4x4 CoronalSliceWidget::getWindowToNIFTIMatrix(bool includeMVP) const
{
    return (getNIFTIToOpenGLMatrix(includeMVP, !includeMVP).inverted() * getWindowToOpenGLMatrix(false, true));
}

QMatrix4x4 CoronalSliceWidget::getWindowToOpenGLMatrix(bool includeMVP, bool flipY) const
{
    QMatrix4x4 windowToOpenGLMatrix;

    windowToOpenGLMatrix.translate(-1.0f, (flipY ? 1.0f : -1.0f));
    windowToOpenGLMatrix.scale(2.0f / width(), (flipY ? -2.0f : 2.0f) / height());

    if (includeMVP)
        return (getMVPMatrix().inverted() * windowToOpenGLMatrix);
    else
        return windowToOpenGLMatrix;
}

QMatrix4x4 CoronalSliceWidget::getNIFTIToOpenGLMatrix(bool includeMVP, bool flipY) const
{
    QMatrix4x4 NIFTIToOpenGLMatrix;

    NIFTIToOpenGLMatrix.translate(-1.0f, (flipY ? 1.0f : -1.0f));
    NIFTIToOpenGLMatrix.scale(2.0f / (fatImage->getXDim() - 1), (flipY ? -2.0f : 2.0f) / (fatImage->getZDim() - 1));

    if (includeMVP)
        return (getMVPMatrix() * NIFTIToOpenGLMatrix);
    else
        return NIFTIToOpenGLMatrix;
}

void CoronalSliceWidget::setUndoStack(QUndoStack *stack)
{
    undoStack = stack;
}

void CoronalSliceWidget::setDirty(int bit)
{
    dirty |= bit;
}

void CoronalSliceWidget::resetView()
{
    // Reset translation and scaling factors
    translation = QVector3D(0.0f, 0.0f, 0.0f);
    scaling = 1.0f;

    // Update the screen
    dirty |= Dirty::Crosshair;
    update();
}

void CoronalSliceWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Setup matrices and view options
    projectionMatrix.setToIdentity();
    projectionMatrix.scale(1.0f, -1.0f, 1.0f);
    viewMatrix.setToIdentity();
    scaling = 1.0f;
    translation = QVector3D(0.0f, 0.0f, 0.0f);

    program = new QOpenGLShaderProgram();
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/coronalslice.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/coronalslice.frag");
    program->link();
    program->bind();

    program->setUniformValue("tex", 0);

    initializeSliceView();
    initializeCrosshairLine();
}

void CoronalSliceWidget::initializeSliceView()
{
    // get context opengl-version
    qDebug() << "----------------- CoronalSliceWidget -------------------------";
    qDebug() << "Widget OpenGL: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid();
    qDebug() << "Really used OpenGL: " << context()->format().majorVersion() << "." << context()->format().minorVersion();
    qDebug() << "OpenGL information: VENDOR:       " << (const char*)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    qDebug() << "";

    sliceTextureInit = false;

    // Setup the axial slice vertices
    sliceVertices.clear();
    sliceVertices.append(VertexPT(QVector3D(-1.0f, -1.0f, 0.0f), QVector2D(0.0f, 0.0f)));
    sliceVertices.append(VertexPT(QVector3D(-1.0f, 1.0f, 0.0f), QVector2D(0.0f, 1.0f)));
    sliceVertices.append(VertexPT(QVector3D(1.0f, -1.0f, 0.0f), QVector2D(1.0f, 0.0f)));
    sliceVertices.append(VertexPT(QVector3D(1.0f, 1.0f, 0.0f), QVector2D(1.0f, 1.0f)));

    // Setup the axial slice indices
    sliceIndices.clear();
    sliceIndices.append({ 0, 1, 2, 3});

    // Generate vertex buffer for the axial slice. The sliceVertices data is uploaded to the VBO
    glGenBuffers(1, &sliceVertexBuf);
    glBindBuffer(GL_ARRAY_BUFFER, sliceVertexBuf);
    glBufferData(GL_ARRAY_BUFFER, sliceVertices.size() * sizeof(VertexPT), sliceVertices.constData(), GL_STATIC_DRAW);

    // Generate index buffer for the axial slice. The sliceIndices data is uploaded to the IBO
    glGenBuffers(1, &sliceIndexBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sliceIndexBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sliceIndices.size() * sizeof(GLushort), sliceIndices.constData(), GL_STATIC_DRAW);

    // Generate VAO for the axial slice vertices uploaded. Location 0 is the position and location 1 is the texture position
    glGenVertexArrays(1, &sliceVertexObject);
    glBindVertexArray(sliceVertexObject);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, VertexPT::PosTupleSize, GL_FLOAT, true, VertexPT::stride(), static_cast<const char *>(0) + VertexPT::posOffset());
    glVertexAttribPointer(1, VertexPT::TexPosTupleSize, GL_FLOAT, true, VertexPT::stride(), static_cast<const char *>(0) + VertexPT::texPosOffset());

    // Generate a blank texture for the axial slice
    glGenTextures(1, &this->sliceTexture);

    // Release (unbind) all
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void CoronalSliceWidget::initializeCrosshairLine()
{
    lineStart = QPoint(0, 0);
    lineEnd = QPoint(0, 0);
    lineWidth = 0;
}

void CoronalSliceWidget::updateTexture()
{
    cv::Mat matrix;
    // Get the slice for the fat image. If the result is empty then there was an error retrieving the slice
    matrix = fatImage->getCoronalSlice(location.y(), true);
    if (matrix.empty())
    {
        qWarning() << "Unable to retrieve coronal slice " << location.y() << " from the fat image. Matrix returned empty.";
        return;
    }

    // The normalize function does quite a bit here. It converts the matrix to a 32-bit float and normalizes it
    // between 0.0f to 1.0f based on the min/max value. This does not affect the original 3D matrix in fatImage
    cv::normalize(matrix.clone(), matrix, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);

    // Bind the texture and setup the parameters for it
    glBindTexture(GL_TEXTURE_2D, sliceTexture);
    // These parameters basically say the pixel value is equal to the average of the nearby pixel values when magnifying or minifying the values
    // Essentially, when stretching or shrinking the texture to the screen, it will smooth out the pixel values instead of making it look blocky
    // like GL_NEAREST would.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glCheckError();

    // Get the OpenGL datatype of the matrix
    auto dataType = NumericType::OpenCV(matrix.type());
    // Upload the texture data from the matrix to the texture. The internal format is 32 bit floats with one channel for red
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, fatImage->getXDim(), fatImage->getZDim(), 0, dataType->openGLFormat, dataType->openGLType, matrix.data);

    // If it hasnt been initialized yet or needs to be reinitialized to a different size, use glTexImage2D, otherwise use
    // the quicker method glTexSubImage2D which just overwrites old data
    if (!sliceTextureInit)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, fatImage->getXDim(), fatImage->getZDim(), 0, dataType->openGLFormat, dataType->openGLType, matrix.data);
        sliceTextureInit = true;
    }
    else
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fatImage->getXDim(), fatImage->getZDim(), dataType->openGLFormat, dataType->openGLType, matrix.data);

    glCheckError();

    dirty &= ~Dirty::Slice;
}

void CoronalSliceWidget::updateCrosshairLine()
{
    // Start with calculating the thickness of each axial layer according to the translation/scaling factors
    // Need to get this in terms of window coordinates b/c that is the system QPainter uses

    // The NIFTI to Window matrix will be used to simply convert from NIFTI coordinates to window coordinates
    QMatrix4x4 NIFTIToWindowMatrix = getWindowToNIFTIMatrix().inverted();

    // Create a start point at top of screen (axialSlice 0)
    // Set the end point to be one slice down (axialSlice 1)
    QVector4D start(0.0f, 0.0f, 0.0f, 1.0f);
    QVector4D end(0.0f, 1.0f, 0.0f, 1.0f);

    // Transform the points from NIFTI coord. system to window coord system.
    // Note: This takes into account the MVP matrix in OpenGL, so any translation,
    // rotation, or scaling is factored in.
    start = NIFTIToWindowMatrix * start;
    end = NIFTIToWindowMatrix * end;

    // Get the difference between the start and end point. The length is the necessary line width to accurately show how
    // large one axial slice is
    QVector4D delta = end - start;

    // Set lineWidth to be an integer value of the delta length. However, the
    // lineWidth must be at least 1 so that the pen will be shown
    lineWidth = std::max((int)std::floor(delta.length()), 1);

    // Start line is left of screen at specified y value and end value is right of screen at specified y value
    // Note: These are in NIFTI coordinate system (0 -> XDim - 1, 0 -> ZDim - 1)
    start = QVector4D(0.0f, location.z(), 0.0f, 1.0f);
    end = QVector4D((fatImage->getXDim() - 1), location.z(), 0.0f, 1.0f);

    // Transform the points to window matrix (factors in OpenGL MVP)
    start = NIFTIToWindowMatrix * start;
    end = NIFTIToWindowMatrix * end;

    // Convert to 2D points
    lineStart = start.toPoint();
    lineEnd = end.toPoint();

    dirty &= ~Dirty::Crosshair;
}

void CoronalSliceWidget::resizeGL(int w, int h)
{
    // Shuts compiler up about unused variables w and h.
    (void)w;
    (void)h;

    // Do nothing if fat/water images are not loaded
    if (!isLoaded())
        return;

    dirty |= Dirty::Crosshair;
    update();
}

void CoronalSliceWidget::paintGL()
{
    // Do nothing if fat/water images are not loaded
    if (!isLoaded())
        return;

    // Update relevant OpenGL objects if dirty
    if (dirty & Dirty::Slice)
        updateTexture();

    if (dirty & Dirty::Crosshair)
        updateCrosshairLine();

    // After updating, begin rendering
    QPainter painter(this);

    painter.beginNativePainting();
    glClear(GL_COLOR_BUFFER_BIT);

    // Calculate the ModelViewProjection (MVP) matrix to transform the location of the axial slices
    QMatrix4x4 mvpMatrix = getMVPMatrix();

    program->bind();
    program->setUniformValue("MVP", mvpMatrix);

    // Bind the VAO, bind texture to GL_TEXTURE0, bind VBO, bind IBO
    // The program that is bound is the index of the curColorMap.
    glBindVertexArray(sliceVertexObject);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sliceTexture);
    glBindBuffer(GL_ARRAY_BUFFER, sliceVertexBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sliceIndexBuf);
    glCheckError();

    // Draw a triangle strip of 4 elements which is two triangles. The indices are unsigned shorts
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
    glCheckError();

    // Release (unbind) the binded objects in reverse order
    // This is a simple protocol to prevent anything happening to the objects outside of this function without
    // explicitly binding the objects
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_1D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    program->release();
    glCheckError();

    painter.endNativePainting();

    // Draw crosshair line
    painter.setPen(QPen(Qt::red, lineWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(lineStart, lineEnd);
}

void CoronalSliceWidget::mouseMoveEvent(QMouseEvent *eventMove)
{
    if (!isLoaded())
        return;

    if (startPan)
    {
        // Change in mouse x/y based on last mouse position
        QPointF curMousePos = eventMove->pos();
        QPointF lastMousePos_ = this->lastMousePos;

        // Get matrix for converting from window to OpenGL coordinate system
        // Note: Do not apply MVP because we do not want to see movement based on
        // scaling (this means dont flip it either)
        QMatrix4x4 windowToOpenGLMatrix = this->getWindowToOpenGLMatrix(false, false);

        // Apply transformation to current and last mouse position
        curMousePos = windowToOpenGLMatrix * curMousePos;
        lastMousePos_ = windowToOpenGLMatrix * lastMousePos_;

        // Get the delta
        QPointF delta = (curMousePos - lastMousePos_);

        // Push a new move command on the undoStack. This will call the command but also keep track
        // of it if an undo or redo action is called. redo function is called immediately.
        undoStack->push(new CoronalMoveCommand(delta, this, moveID));

        // Set last mouse position to this one
        lastMousePos = eventMove->pos();
    }
}

void CoronalSliceWidget::mousePressEvent(QMouseEvent *eventPress)
{
    if (!isLoaded())
        return;

    // There are button() and buttons() functions in QMouseEvent that have a very
    // important distinction between one another. button() is only going to return
    // the button that caused this event while buttons() is a state of all buttons
    // held down during this event.
    // Since multiple buttons down does not have functionality, the button() function
    // is used and if an additional button is down with functionality, the old button
    // is turned off and the new takes place.
    if (eventPress->button() == Qt::LeftButton || eventPress->button() == Qt::MiddleButton)
    {
        // Flag to indicate that panning is occuring
        // The starting position is stored so to know how much movement has occurred
        startPan = true;
        moveID = (((int)moveID + 1) > (int)CommandID::CoronalMoveEnd) ? CommandID::CoronalMove : (CommandID)((int)moveID + 1);
        lastMousePos = eventPress->pos();
    }
}

void CoronalSliceWidget::mouseReleaseEvent(QMouseEvent *eventRelease)
{
    if (!isLoaded())
        return;

    if ((eventRelease->button() == Qt::LeftButton || eventRelease->button() == Qt::MiddleButton) && startPan)
    {
        startPan = false;
    }
}

void CoronalSliceWidget::wheelEvent(QWheelEvent *event)
{
    // If a mouse button is down while doing this event, do not scale
    if (event->buttons() != Qt::NoButton)
        return;

    // The unit for angle delta is in eighths of a degree
    QPoint numDegrees = event->angleDelta() / 8;

    if (!numDegrees.isNull())
    {
        // Zoom in 5% every 15 degrees which is one step on most mouses
        float scaleDelta = numDegrees.y() * (0.05f / 15);

        // Push a new scale command on the undo stack which will immediately call redo for an action.
        // This keeps track of it if an undo or redo command is called
        undoStack->push(new CoronalScaleCommand(scaleDelta, this));
    }
}

CoronalSliceWidget::~CoronalSliceWidget()
{
    // Destroy the VAO, VBO, and IBO
    glDeleteVertexArrays(1, &sliceVertexObject);
    glDeleteBuffers(1, &sliceVertexBuf);
    glDeleteBuffers(1, &sliceIndexBuf);
    glDeleteTextures(1, &sliceTexture);
    delete program;
}
