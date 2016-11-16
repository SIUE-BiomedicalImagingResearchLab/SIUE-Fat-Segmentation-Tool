#include "axialfatslicewidget.h"

AxialFatSliceWidget::AxialFatSliceWidget(QWidget *parent) : QOpenGLWidget(parent)
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

    this->currSlice = 1;
    this->currTime = -1;
}

void AxialFatSliceWidget::setImages(NIFTImage *fat, NIFTImage *water)
{
    if (!fat || !water)
        return;

    fatImage = fat;
    waterImage = water;

    SetAxialSlice(54);
    // TODO: Make this a better value
}

void AxialFatSliceWidget::SetAxialSlice(int slice, int time)
{
    if (!fatImage || !waterImage)
        return;

    cv::Mat matrix;
    switch (displayType)
    {
        case AxialDisplayType::FatOnly:
        {
            matrix = fatImage->getSlice(slice);
            if (matrix.empty())
            {
                qDebug() << "Unable to retrieve slice " << slice << " from the fat image. Matrix returned empty.";
                return;
            }

            cv::normalize(matrix.clone(), matrix, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);
        }
        break;

        case AxialDisplayType::WaterOnly:
        {
            matrix = waterImage->getSlice(slice, true);
            if (matrix.empty())
            {
                qDebug() << "Unable to retrieve slice " << slice << " from the fat image. Matrix returned empty.";
                return;
            }

            cv::normalize(matrix.clone(), matrix, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);
        }
        break;

        case AxialDisplayType::FatFraction:
        {
            cv::Mat fatTemp = fatImage->getSlice(slice, true);
            cv::Mat waterTemp = waterImage->getSlice(slice, true);
            if (fatTemp.empty() || waterTemp.empty())
            {
                qDebug() << "Unable to retrieve slice " << slice << " from the fat or water image. Matrix returned empty.";
                return;
            }

            cv::normalize(fatTemp.clone(), fatTemp, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);
            cv::normalize(waterTemp.clone(), waterTemp, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);

            matrix = (fatTemp - waterTemp) / fatTemp;
        }
        break;

        case AxialDisplayType::WaterFraction:
        {
            cv::Mat fatTemp = fatImage->getSlice(slice, true);
            cv::Mat waterTemp = waterImage->getSlice(slice, true);
            if (fatTemp.empty() || waterTemp.empty())
            {
                qDebug() << "Unable to retrieve slice " << slice << " from the fat or water image. Matrix returned empty.";
                return;
            }

            cv::normalize(fatTemp.clone(), fatTemp, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);
            cv::normalize(waterTemp.clone(), waterTemp, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);

            matrix = (waterTemp - fatTemp) / waterTemp;
        }
        break;
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLenum *dataType = NIFTImage::openCVToOpenGLDatatype(matrix.type());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, fatImage->getXDim(), fatImage->getYDim(), 0, dataType[1], dataType[2], matrix.data);

    GLenum err;
    if ((err = glGetError()) != GL_NO_ERROR)
        qDebug() << "Unable to upload texture image for slice " << slice << ". Error code: " << err;

    update();
}

void AxialFatSliceWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

    // Create a new shader variable, load the GLSL vertex and fragment files, link and bind the program.
    program = new QOpenGLShaderProgram();
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/axialslice.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/axialslice.frag");
    program->link();
    program->bind();

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
    program->setUniformValue("tex", 0);

    // Release (unbind) all
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    program->release();
}

void AxialFatSliceWidget::resizeGL(int w, int h)
{
    // Update projection matrix and other size related settings:
    //m_projection.setToIdentity();
    //m_projection.perspective(45.0f, w / float(h), 0.01f, 100.0f);

    /* I am on hold with this for awhile until I can find out how to draw the 3D image of the NIFTI file onto a OpenGL surface.
     * I am not sure what to do about the vertices and elements buffer since I only have a file that says what the color intensity
     * should be at a certain point */
}

void AxialFatSliceWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Bind the shader program, VAO, bind texture to GL_TEXTURE0, bind VBO, bind IBO
    program->bind();
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
    program->release();
}

void AxialFatSliceWidget::mouseMoveEvent(QMouseEvent *eventMove)
{
    //qDebug() << "MOVE!";
}

void AxialFatSliceWidget::mousePressEvent(QMouseEvent *eventPress)
{
    //qDebug() << "PRESS";
}

void AxialFatSliceWidget::mouseReleaseEvent(QMouseEvent *eventRelease)
{
    //qDebug() << "RELEASE";
}

AxialFatSliceWidget::~AxialFatSliceWidget()
{
    // Destroy the VAO, VBO, and IBO
    glDeleteVertexArrays(1, &vertexObject);
    glDeleteBuffers(1, &vertexBuf);
    glDeleteBuffers(1, &indexBuf);
    delete program;
}
