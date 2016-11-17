#include "axialslicewidget.h"

AxialSliceWidget::AxialSliceWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    this->displayType = AxialDisplayType::WaterOnly;//AxialDisplayType::FatOnly;

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
}

void AxialSliceWidget::setImages(NIFTImage *fat, NIFTImage *water)
{
    if (!fat || !water)
        return;

    fatImage = fat;
    waterImage = water;

    this->curSlice = -1;
    this->curTime = -1;
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
            qDebug() << "Testing...";
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

    for (int i = 0; i < ColorMap::Count; ++i)
    {
        program[i]->bind();
        program[i]->setUniformValue("contrast", contrast);
    }

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

    for (int i = 0; i < ColorMap::Count; ++i)
    {
        program[i]->bind();
        program[i]->setUniformValue("brightness", brightness);
    }

    // Redraw the screen because the brightness has changed
    update();
}

void AxialSliceWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

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
    // Update projection matrix and other size related settings:
    //m_projection.setToIdentity();
    //m_projection.perspective(45.0f, w / float(h), 0.01f, 100.0f);

    /* I am on hold with this for awhile until I can find out how to draw the 3D image of the NIFTI file onto a OpenGL surface.
     * I am not sure what to do about the vertices and elements buffer since I only have a file that says what the color intensity
     * should be at a certain point */
}

void AxialSliceWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Bind the shader program, VAO, bind texture to GL_TEXTURE0, bind VBO, bind IBO
    // The program that is bound is the index of the curColorMap.
    program[(int)curColorMap]->bind();
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
    program[(int)curColorMap]->release();
}

void AxialSliceWidget::mouseMoveEvent(QMouseEvent *eventMove)
{
    //qDebug() << "MOVE!";
}

void AxialSliceWidget::mousePressEvent(QMouseEvent *eventPress)
{
    //qDebug() << "PRESS";
}

void AxialSliceWidget::mouseReleaseEvent(QMouseEvent *eventRelease)
{
    //qDebug() << "RELEASE";
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
