#include "axialslicewidget.h"

AxialSliceWidget::AxialSliceWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    this->displayType = AxialDisplayType::FatOnly;

    this->fatImage = NULL;
    this->waterImage = NULL;

    this->texture = NULL;
    this->sliceVertices.append(VertexPT(QVector3D(-1.0f, -1.0f, 0.0f), QVector2D(0.0f, 0.0f)));
    this->sliceVertices.append(VertexPT(QVector3D(-1.0f, 1.0f, 0.0f), QVector2D(0.0f, 1.0f)));
    this->sliceVertices.append(VertexPT(QVector3D(1.0f, -1.0f, 0.0f), QVector2D(1.0f, 0.0f)));
    this->sliceVertices.append(VertexPT(QVector3D(1.0f, 1.0f, 0.0f), QVector2D(1.0f, 1.0f)));
    this->sliceIndices.append({ 0, 1, 2, 3});

    this->curSlice = 1;
    this->curTime = -1;
    this->curColorMap = ColorMap::Gray;
    this->curBrightness = 0.0f;
    this->curContrast = 1.0f;

    this->startDraw = false;
    this->startPan = false;

    this->projectionMatrix.setToIdentity();
    this->viewMatrix.setToIdentity();
    this->scaling = 1.0f;
    this->translation = QVector3D(0.0f, 0.0f, 0.0f);
}

void AxialSliceWidget::setImages(NIFTImage *fat, NIFTImage *water)
{
    if (!fat || !water)
        return;

    fatImage = fat;
    waterImage = water;

    this->curSlice = -1;
    this->curTime = -1;

    this->projectionMatrix.setToIdentity();
    this->viewMatrix.setToIdentity();
    this->scaling = 1.0f;
    this->translation = QVector3D(0.0f, 0.0f, 0.0f);
}

void AxialSliceWidget::setAxialSlice(int slice, int time)
{
    // If there is no fat or water image currently loaded or the slice is not in an acceptable range,
    // then return with doing nothing.
    if (!fatImage || !waterImage || slice < 0 || slice >= fatImage->getZDim())
        return;

    // Update the currSlice and currTime variables
    this->curSlice = slice;
    this->curTime =  time;

    cv::Mat matrix;
    switch (displayType)
    {
        case AxialDisplayType::FatOnly:
        {
            // Get the slice for the fat image. If the result is empty then there was an error retrieving the slice
            matrix = fatImage->getSlice(slice);
            if (matrix.empty())
            {
                qDebug() << "Unable to retrieve slice " << slice << " from the fat image. Matrix returned empty.";
                return;
            }

            // The normalize function does quite a bit here. It converts the matrix to a 32-bit float and normalizes it
            // between 0.0f to 1.0f based on the min/max value. This does not affect the original 3D matrix in fatImage
            cv::normalize(matrix.clone(), matrix, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);
        }
        break;

        case AxialDisplayType::WaterOnly:
        {
            // Get the slice for the water image. If the result is empty then there was an error retrieving the slice
            matrix = waterImage->getSlice(slice, true);
            if (matrix.empty())
            {
                qDebug() << "Unable to retrieve slice " << slice << " from the fat image. Matrix returned empty.";
                return;
            }

            // The normalize function does quite a bit here. It converts the matrix to a 32-bit float and normalizes it
            // between 0.0f to 1.0f based on the min/max value. This does not affect the original 3D matrix in waterImage
            cv::normalize(matrix.clone(), matrix, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);
        }
        break;

        case AxialDisplayType::FatFraction:
        {
            // Get the slice for the fat/water image. If the result is empty then there was an error retrieving the slice
            cv::Mat fatTemp = fatImage->getSlice(slice, true);
            cv::Mat waterTemp = waterImage->getSlice(slice, true);
            if (fatTemp.empty() || waterTemp.empty())
            {
                qDebug() << "Unable to retrieve slice " << slice << " from the fat or water image. Matrix returned empty.";
                return;
            }

            // The normalize function does quite a bit here. It converts the matrix to a 32-bit float and normalizes it
            // between 0.0f to 1.0f based on the min/max value. This does not affect the original 3D matrix in fatImage/waterImage
            cv::normalize(fatTemp.clone(), fatTemp, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);
            cv::normalize(waterTemp.clone(), waterTemp, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);

            matrix = fatTemp / (fatTemp + waterTemp);
        }
        break;

        case AxialDisplayType::WaterFraction:
        {
            // Get the slice for the fat/water image. If the result is empty then there was an error retrieving the slice
            cv::Mat fatTemp = fatImage->getSlice(slice, true);
            cv::Mat waterTemp = waterImage->getSlice(slice, true);
            if (fatTemp.empty() || waterTemp.empty())
            {
                qDebug() << "Unable to retrieve slice " << slice << " from the fat or water image. Matrix returned empty.";
                return;
            }

            // The normalize function does quite a bit here. It converts the matrix to a 32-bit float and normalizes it
            // between 0.0f to 1.0f based on the min/max value. This does not affect the original 3D matrix in fatImage/waterImage
            cv::normalize(fatTemp.clone(), fatTemp, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);
            cv::normalize(waterTemp.clone(), waterTemp, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);

            matrix = waterTemp / (fatTemp + waterTemp);
        }
        break;
    }

    // Bind the texture and setup the parameters for it
    glBindTexture(GL_TEXTURE_2D, texture);
    // These parameters basically say the pixel value is equal to the average of the nearby pixel values when magnifying or minifying the values
    // Essentially, when stretching or shrinking the texture to the screen, it will smooth out the pixel values instead of making it look blocky
    // like GL_NEAREST would.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Get the OpenGL datatype of the matrix
    GLenum *dataType = NIFTImage::openCVToOpenGLDatatype(matrix.type());
    // Upload the texture data from the matrix to the texture. The internal format is 32 bit floats with one channel for red
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, fatImage->getXDim(), fatImage->getYDim(), 0, dataType[1], dataType[2], matrix.data);

    // If there was an error, then say something
    GLenum err;
    if ((err = glGetError()) != GL_NO_ERROR)
        qDebug() << "Unable to upload texture image for slice " << slice << ". Error code: " << err;

    // This update tells the widget to redraw itself since the texture was updated
    update();
}

AxialDisplayType AxialSliceWidget::getDisplayType()
{
    return displayType;
}

void AxialSliceWidget::setDisplayType(AxialDisplayType type)
{
    // If the display type is out of the acceptable range, then do nothing
    if (type < AxialDisplayType::FatOnly || type > AxialDisplayType::WaterFraction)
    {
        qDebug() << "Invalid display type was specified for AxialSliceWidget: " << type;
        return;
    }

    displayType = type;

    // Call setAxialSlice which will update the texture with the appropiate data and redraw the screen
    setAxialSlice(curSlice, curTime);
}

ColorMap AxialSliceWidget::getColorMap()
{
    return curColorMap;
}

void AxialSliceWidget::setColorMap(ColorMap map)
{
    // If the map given is out of the acceptable range, then do nothing
    if (map < ColorMap::Autumn || map >= ColorMap::Count)
    {
        qDebug() << "Invalid color map was specified for AxialSliceWidget: " << map;
        return;
    }

    curColorMap = map;

    // Redraw the screen because the screen colormap has changed
    update();
}

float AxialSliceWidget::getContrast()
{
    return curContrast;
}

void AxialSliceWidget::setContrast(float contrast)
{
    // If the contrast is out of the acceptable range, then do nothing
    if (contrast < 0.0f || contrast > 1.0f)
    {
        qDebug() << "Invalid contrast was specified for AxialSliceWidget: " << contrast;
        return;
    }

    curContrast = contrast;

    // Redraw the screen because the contrast has changed
    update();
}

float AxialSliceWidget::getBrightness()
{
    return curBrightness;
}

void AxialSliceWidget::setBrightness(float brightness)
{
    // If the brightness is out of the acceptable range, then do nothing
    if (brightness < 0.0f || brightness > 1.0f)
    {
        qDebug() << "Invalid brightness was specified for AxialSliceWidget: " << brightness;
        return;
    }

    curBrightness = brightness;

    // Redraw the screen because the brightness has changed
    update();
}

void AxialSliceWidget::resetView()
{
    // Reset translation and scaling factors
    translation = QVector3D(0.0f, 0.0f, 0.0f);
    scaling = 1.0f;

    // Update the screen
    update();
}

void AxialSliceWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Create MVP matrix from the model, view, and projection matrix.
    QMatrix4x4 mvp = /* model * */viewMatrix * projectionMatrix;

    // Create a new shader variable, load the GLSL vertex and fragment files, link and bind the program.
    for (int i = 0; i < ColorMap::Count; ++i)
    {
        program[i] = new QOpenGLShaderProgram();
        program[i]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/axialslice.vert");
        program[i]->addShaderFromSourceFile(QOpenGLShader::Fragment, colorMapFragName[i]);
        program[i]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/axialslice.frag");
        program[i]->link();
        program[i]->bind();

        program[i]->setUniformValue("tex", 0);
        program[i]->setUniformValue("mvp", mvp);
    }

    // Generate vertex buffer for the axial slice. The sliceVertices data is uploaded to the VBO
    glGenBuffers(1, &vertexBuf);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
    glBufferData(GL_ARRAY_BUFFER, sliceVertices.size() * sizeof(VertexPT), sliceVertices.constData(), GL_STATIC_DRAW);

    // Generate index buffer for the axial slice. The sliceIndices data is uploaded to the IBO
    glGenBuffers(1, &indexBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sliceIndices.size() * sizeof(GLushort), sliceIndices.constData(), GL_STATIC_DRAW);

    // Generate VAO for the axial slice vertices uploaded. Location 0 is the position and location 1 is the texture position
    glGenVertexArrays(1, &vertexObject);
    glBindVertexArray(vertexObject);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, VertexPT::PosTupleSize, GL_FLOAT, true, VertexPT::stride(), static_cast<const char *>(0) + VertexPT::posOffset());
    glVertexAttribPointer(1, VertexPT::TexPosTupleSize, GL_FLOAT, true, VertexPT::stride(), static_cast<const char *>(0) + VertexPT::texPosOffset());

    // Enable 2D textures and generate a blank texture for the axial slice
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &this->texture);
    // This sets the GLSL uniform value 'tex' to 0 which means GL_TEXTURE0 is the texture used for tex variable

    // Release (unbind) all
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void AxialSliceWidget::resizeGL(int w, int h)
{
    // Shuts compiler up about unused variables w and h.
    (void)w;
    (void)h;

    /* Because of the simplicity of this program, A.K.A because it is in 2D, there is no need to do anything when the window itself is resized.
     * This is automatically handled and it is typically only useful when doing 3D applications I believe.
     */
}

void AxialSliceWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Calculate the ModelViewProjection (MVP) matrix to transform the location of the axial slices
    QMatrix4x4 mvpMatrix;
    QMatrix4x4 modelMatrix;

    // Translate the modelMatrix according to translation vector
    // Then scale according to scaling factor
    modelMatrix.translate(translation);
    modelMatrix.scale(scaling);

    // Create the MVP matrix by M * V * P
    mvpMatrix = modelMatrix * viewMatrix * projectionMatrix;

    // Get the current shader program, bind it, and set the brightness, contrast, and MVP
    // Note: I did quite a bit of research on this and updating the uniform values in this function is
    // likely going to be the best results. Yes, it may be redundant and slow down the program when it
    // doesn't change much but when the program is quickly working, it will be updating this regardless
    // Until I see performance issues with this method, I believe it is the best method
    QOpenGLShaderProgram *curProgram = program[(int)curColorMap];
    curProgram->bind();
    curProgram->setUniformValue("brightness", curBrightness);
    curProgram->setUniformValue("contrast", curContrast);
    curProgram->setUniformValue("MVP", mvpMatrix);

    // Bind the VAO, bind texture to GL_TEXTURE0, bind VBO, bind IBO
    // The program that is bound is the index of the curColorMap.
    glBindVertexArray(vertexObject);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);

    // Draw a triangle strip of 4 elements which is two triangles. The indices are unsigned shorts
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);

    // Release (unbind) the binded objects in reverse order
    // This is a simple protocol to prevent anything happening to the objects outside of this function without
    // explicitly binding the objects
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    curProgram->release();
}

void AxialSliceWidget::mouseMoveEvent(QMouseEvent *eventMove)
{
    if (startDraw)
    {
        // Do drawing stuff
    }
    else if (startPan)
    {
        // Change in mouse x/y based on last mouse position
        QPointF delta = (eventMove->pos() - lastMousePos);

        // Divide delta by respective width/height of screen and add it to the translation
        translation += QVector3D((delta.x() * 2.0f / this->width()), (-delta.y() * 2.0f / this->height()), 0.0f);

        // Tell the screen to draw itself since the scene changed
        update();

        // Set last mouse position to this one
        lastMousePos = eventMove->pos();
    }
}

void AxialSliceWidget::mousePressEvent(QMouseEvent *eventPress)
{
    // There are button() and buttons() functions in QMouseEvent that have a very
    // important distinction between one another. button() is only going to return
    // the button that caused this event while buttons() is a state of all buttons
    // held down during this event.
    // Since multiple buttons down does not have functionality, the button() function
    // is used and if an additional button is down with functionality, the old button
    // is turned off and the new takes place.
    if (eventPress->button() == Qt::LeftButton)
    {
        // See below for explanation of why this occurs
        if (startPan)
        {
            // TODO: Any other stuff here
            startPan = false;
        }

        // Start drawing stuff here
        startDraw = true;
    }
    else if (eventPress->button() == Qt::MiddleButton)
    {
        // If the left button is already down and the middle button is clicked, we want
        // to turn off the drawing functionality from the left click and start panning.
        // Basically, we don't want to be drawing and panning with the Left/Middle clicks
        // both on so the latest one overwrites
        if (startDraw)
        {
            // TODO: Any other stuff here
            startDraw = false;
        }

        // Flag to indicate that panning is occuring
        // The starting position is stored so to know how much movement has occurred
        startPan = true;
        lastMousePos = eventPress->pos();
    }
}

void AxialSliceWidget::mouseReleaseEvent(QMouseEvent *eventRelease)
{
    if (eventRelease->button() == Qt::LeftButton && startDraw)
    {
        // Stop drawing here
        startDraw = false;
    }
    else if (eventRelease->button() == Qt::MiddleButton && startPan)
    {
        startPan = false;
    }
}

void AxialSliceWidget::wheelEvent(QWheelEvent *event)
{
    // The unit for angle delta is in eighths of a degree
    QPoint numDegrees = event->angleDelta() / 8;

    if (!numDegrees.isNull())
    {
        // Zoom in 5% every 15 degrees which is one step on most mouses
        scaling += numDegrees.y() * (0.05f / 15);

        // Clamp it between 0.05f (5%) to 3.0f (300%)
        scaling = std::max(std::min(scaling, 3.0f), 0.05f);

        // Update the screen
        update();
    }
}

AxialSliceWidget::~AxialSliceWidget()
{
    // Destroy the VAO, VBO, and IBO
    glDeleteVertexArrays(1, &vertexObject);
    glDeleteBuffers(1, &vertexBuf);
    glDeleteBuffers(1, &indexBuf);
    for (int i = 0; i < ColorMap::Count; ++i)
        delete program[i];
}
