#include "axialslicewidget.h"
#include "commands.h"

AxialSliceWidget::AxialSliceWidget(QWidget *parent) : QOpenGLWidget(parent),
    displayType(SliceDisplayType::FatOnly), fatImage(NULL), waterImage(NULL),
    tracingLayerColors({ Qt::blue, Qt::darkCyan, Qt::cyan, Qt::magenta, Qt::yellow, Qt::green }), mouseCommand(NULL),
    slicePrimTexture(NULL), sliceSecdTexture(NULL),
    location(0, 0, 0, 0), locationLabel(NULL), primColorMap(ColorMap::Gray), primOpacity(1.0f), secdColorMap(ColorMap::Gray), secdOpacity(1.0f),
    brightness(0.0f), contrast(1.0f), tracingLayer(TracingLayer::EAT),
    startDraw(false), startPan(false), moveID(CommandID::AxialMove)
{
    this->tracingLayerVisible.fill(true);
}

void AxialSliceWidget::setImages(NIFTImage *fat, NIFTImage *water)
{
    if (!fat || !water)
        return;

    fatImage = fat;
    waterImage = water;

    location = QVector4D(0, 0, 0, 0);

    // Resize points vector to accomodate layers and axial slices
    this->points.resize(fatImage->getZDim());
    for (auto &layers : points)
        layers.resize((int)TracingLayer::Count);
}

bool AxialSliceWidget::isLoaded() const
{
    return (fatImage && waterImage);
}

void AxialSliceWidget::setLocation(QVector4D location)
{
    // If there is no fat or water image currently loaded then return with doing nothing.
    if (!isLoaded())
        return;

    QVector4D transformedLocation = transformLocation(location);
    QVector4D delta = transformedLocation - this->location;

    this->location = transformedLocation;

    // If Y value changed, then update the crosshair line
    if (delta.y())
        updateCrosshairLine();

    // If Z value changed, then update the texture
    if (delta.z())
        updateTexture();

    // Update location label
    if (locationLabel)
        locationLabel->setText(QObject::tr("Location: (%1, %2, %3)").arg(this->location.x()).arg(this->location.y()).arg(this->location.z()));
}

QVector4D AxialSliceWidget::getLocation() const
{
    return location;
}

QVector4D AxialSliceWidget::transformLocation(QVector4D location) const
{
    // This function returns a QVector4D that replaces Location::NoChange's from location variable with actual location value

    QVector4D temp(location.x() == Location::NoChange, location.y() == Location::NoChange, location.z() == Location::NoChange, location.w() == Location::NoChange);
    QVector4D notTemp = QVector4D(1.0f, 1.0f, 1.0f, 1.0f) - temp;
    return (location * notTemp) + (this->location * temp);
}

QLabel *AxialSliceWidget::getLocationLabel() const
{
    return locationLabel;
}

void AxialSliceWidget::setLocationLabel(QLabel *label)
{
    locationLabel = label;
}

SliceDisplayType AxialSliceWidget::getDisplayType() const
{
    return displayType;
}

void AxialSliceWidget::setDisplayType(SliceDisplayType type)
{
    // If the display type is out of the acceptable range, then do nothing
    if (type < SliceDisplayType::FatOnly || type > SliceDisplayType::WaterFat)
    {
        qDebug() << "Invalid display type was specified for AxialSliceWidget: " << (int)type;
        return;
    }

    displayType = type;

    // This will recreate the texture because the display type has changed
    updateTexture();
}

ColorMap AxialSliceWidget::getPrimColorMap() const
{
    return primColorMap;
}

void AxialSliceWidget::setPrimColorMap(ColorMap map)
{
    // If the map given is out of the acceptable range, then do nothing
    if (map < ColorMap::Autumn || map >= ColorMap::Count)
    {
        qDebug() << "Invalid primary color map was specified for AxialSliceWidget: " << (int)map;
        return;
    }

    primColorMap = map;

    // Redraw the screen because the screen colormap has changed
    update();
}

float AxialSliceWidget::getPrimOpacity() const
{
    return primOpacity;
}

void AxialSliceWidget::setPrimOpacity(float opacity)
{
    if (opacity < 0.0f || opacity > 1.0f)
    {
        qDebug() << "Invalid primary opacity level was specified for AxialSliceWidget: " << opacity;
        return;
    }

    primOpacity = opacity;

    // Redraw the screen because the opacity of one of the objects changed
    update();
}

ColorMap AxialSliceWidget::getSecdColorMap() const
{
    return secdColorMap;
}

void AxialSliceWidget::setSecdColorMap(ColorMap map)
{
    // If the map given is out of the acceptable range, then do nothing
    if (map < ColorMap::Autumn || map >= ColorMap::Count)
    {
        qDebug() << "Invalid secondary color map was specified for AxialSliceWidget: " << (int)map;
        return;
    }

    secdColorMap = map;

    // Redraw the screen because the screen colormap has changed
    update();
}

float AxialSliceWidget::getSecdOpacity() const
{
    return secdOpacity;
}

void AxialSliceWidget::setSecdOpacity(float opacity)
{
    if (opacity < 0.0f || opacity > 1.0f)
    {
        qDebug() << "Invalid secondary opacity level was specified for AxialSliceWidget: " << opacity;
        return;
    }

    secdOpacity = opacity;

    // Redraw the screen because the opacity of one of the objects changed
    update();
}

float AxialSliceWidget::getBrightness() const
{
    return brightness;
}

void AxialSliceWidget::setBrightness(float brightness)
{
    // If the brightness is out of the acceptable range, then do nothing
    if (brightness < 0.0f || brightness > 1.0f)
    {
        qDebug() << "Invalid brightness was specified for AxialSliceWidget: " << brightness;
        return;
    }

    this->brightness = brightness;

    // Redraw the screen because the brightness has changed
    update();
}

float AxialSliceWidget::getContrast() const
{
    return contrast;
}

void AxialSliceWidget::setContrast(float contrast)
{
    // If the contrast is out of the acceptable range, then do nothing
    if (contrast < 0.0f || contrast > 1.0f)
    {
        qDebug() << "Invalid contrast was specified for AxialSliceWidget: " << contrast;
        return;
    }

    this->contrast = contrast;

    // Redraw the screen because the contrast has changed
    update();
}

TracingLayer AxialSliceWidget::getTracingLayer() const
{
    return tracingLayer;
}

void AxialSliceWidget::setTracingLayer(TracingLayer layer)
{
    // If the layer is out of the acceptable range, then do nothing
    if (layer < TracingLayer::EAT || layer >= TracingLayer::Count)
    {
        qDebug() << "Invalid current tracing layer was specified for AxialSliceWidget: " << (int)layer;
        return;
    }

    tracingLayer = layer;
}

bool AxialSliceWidget::getTracingLayerVisible(TracingLayer layer) const
{
    // If the layer is out of the acceptable range, then do nothing
    if (layer < TracingLayer::EAT || layer >= TracingLayer::Count)
    {
        qDebug() << "Invalid current tracing layer was specified for AxialSliceWidget: " << (int)layer;
        return false;
    }

    return tracingLayerVisible[(int)layer];
}

void AxialSliceWidget::setTracingLayerVisible(TracingLayer layer, bool value)
{
    // If the layer is out of the acceptable range, then do nothing
    if (layer < TracingLayer::EAT || layer >= TracingLayer::Count)
    {
        qDebug() << "Invalid current tracing layer was specified for AxialSliceWidget: " << (int)layer;
        return;
    }

    tracingLayerVisible[(int)layer] = value;
}

std::vector<std::vector<QPointF>> &AxialSliceWidget::getSlicePoints(int slice)
{
    if (slice == Location::NoChange)
        slice = location.z();
    else if (slice < 0 || slice >= fatImage->getZDim())
    {
        qDebug() << "Invalid slice given for getSlicePoints in AxialSliceWidget: " << slice;
        slice = 0; // Since a reference is returned, just set a dummy slice
    }

    return points[slice];
}

std::vector<QPointF> &AxialSliceWidget::getLayerPoints(int slice, TracingLayer layer)
{
    if (slice == Location::NoChange)
        slice = location.z();
    else if (slice < 0 || slice >= fatImage->getZDim())
    {
        qDebug() << "Invalid slice given for getLayerPoints in AxialSliceWidget: " << slice;
        slice = 0; // Since a reference is returned, just set a dummy slice
    }

    if (layer == TracingLayer::Count)
        layer = tracingLayer;
    else if (layer < TracingLayer::EAT || layer > TracingLayer::Count)
    {
        qDebug() << "Invalid tracing layer given for getLayerPoints in AxialSliceWidget: " << (int)layer;
        layer = TracingLayer::EAT; // Since a reference is returned, just set a dummy layer
    }

    return points[slice][(int)layer];
}

float &AxialSliceWidget::rscaling()
{
    return scaling;
}

QVector3D &AxialSliceWidget::rtranslation()
{
    return translation;
}

bool AxialSliceWidget::saveTracingData(QString path, bool promptOnOverwrite)
{
    const QString layerFilename[(int)TracingLayer::Count] = {"EAT.txt", "IMAT.txt", "PAAT.txt", "PAT.txt", "SCAT.txt", "VAT.txt"};
    QString layerFullPath[(int)TracingLayer::Count];

    // Create absolute path to each of the layer filenames by joining directory path (path) with each filename (layerFilename)
    for (int i = 0; i < (int)TracingLayer::Count; ++i)
        layerFullPath[i] = QDir(path).filePath(layerFilename[i]);

    if (promptOnOverwrite)
    {
        for (QString fullPath : layerFullPath)
        {
            if (QFileInfo(fullPath).exists())
            {
                // Not a fan of the prompt option
                if (!QMessageBox::warning((QWidget *)parent(), "Confirm Save As", "Tracing data already exists in folder.\nDo you want to replace it?", QMessageBox::Yes, QMessageBox::No))
                    return false;
                break;
            }
        }
    }

    for (int i = 0; i < (int)TracingLayer::Count; ++i)
    {
        QFile file(layerFullPath[i]);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            qDebug() << "Error opening file to save tracing data. Skipping layer: " << layerFullPath[i];
            continue;
        }

        QTextStream stream(&file);
        stream << fatImage->getZDim() << endl;

        const int zDim = fatImage->getZDim();
        for (int z = 0; z < zDim; ++z)
        {
            auto &layerPoints = getLayerPoints(z, (TracingLayer)i);

            stream << "#" << z << endl;
            stream << layerPoints.size() << endl;

            for (QPointF point : layerPoints)
                stream << forcepoint << point.x() << " " << point.y() << " " << (float)z << endl;
        }
    }

    return true;
}


bool AxialSliceWidget::loadTracingData(QString path)
{
    if (!isLoaded())
    {
        QMessageBox::warning((QWidget *)parent(), "Unable to import data", "Tracing data cannot be imported into an application until the correct NIFTI image is loaded first.\nPlease load the correct NIFTI file and then try again.", QMessageBox::Ok);
        return false;
    }

    const QString layerFilename[(int)TracingLayer::Count] = {"EAT.txt", "IMAT.txt", "PAAT.txt", "PAT.txt", "SCAT.txt", "VAT.txt"};
    QString layerFullPath[(int)TracingLayer::Count];
    std::vector<int> skipLayers;

    // Create absolute path to each of the layer filenames by joining directory path (path) with each filename (layerFilename)
    for (int i = 0; i < (int)TracingLayer::Count; ++i)
    {
        layerFullPath[i] = QDir(path).filePath(layerFilename[i]);

        if (!QFileInfo(layerFullPath[i]).exists())
        {
            qDebug() << "Filename does not exist in directory for importing tracing data: " << layerFullPath[i];
            skipLayers.push_back(i);
        }
    }

    for (int i = 0; i < (int)TracingLayer::Count; ++i)
    {
        bool nextLayer = false;
        for (int skipLayer : skipLayers)
        {
            if (i == skipLayer)
            {
                nextLayer = true;
                break;
            }
        }

        if (nextLayer)
            continue;

        QFile file(layerFullPath[i]);
        if (!file.open(QIODevice::ReadOnly))
        {
            qDebug() << "Error opening file to load tracing data. Skipping layer: " << layerFullPath[i];
            continue;
        }

        QTextStream stream(&file);

        int zDim;
        stream >> zDim;

        if (zDim != fatImage->getZDim())
        {
            qDebug() << "Number of axial slices in the data does not match the NIFTI image loaded.";
            return false;
        }

        for (int z = 0; z < zDim; ++z)
        {
            auto &layerPoints = getLayerPoints(z, (TracingLayer)i);

            // Skip the #Z where Z is the axial slice
            stream.skipWhiteSpace();
            stream.readLine();

            // Get the number of points on the slices
            int numPoints = 0;
            stream >> numPoints;

            // Clear the old points and reserve enough space for new points
            layerPoints.clear();
            layerPoints.reserve(numPoints);

            float x, y, z_;
            for (int ii = 0; ii < numPoints; ++ii)
            {
                stream >> x >> y >> z_;
                layerPoints.push_back(QPoint(x, y));
            }
        }
    }

    return true;
}

QMatrix4x4 AxialSliceWidget::getMVPMatrix() const
{
    // Calculate the ModelViewProjection (MVP) matrix to transform the location of the axial slices
    QMatrix4x4 modelMatrix;

    // Translate the modelMatrix according to translation vector
    // Then scale according to scaling factor
    modelMatrix.translate(translation);
    modelMatrix.scale(scaling);

    return (projectionMatrix * viewMatrix * modelMatrix);
}

QMatrix4x4 AxialSliceWidget::getWindowToNIFTIMatrix(bool includeMVP) const
{
    return (getNIFTIToOpenGLMatrix(includeMVP, !includeMVP).inverted() * getWindowToOpenGLMatrix(false, true));
}

QMatrix4x4 AxialSliceWidget::getWindowToOpenGLMatrix(bool includeMVP, bool flipY) const
{
    QMatrix4x4 windowToOpenGLMatrix;

    windowToOpenGLMatrix.translate(-1.0f, (flipY ? 1.0f : -1.0f));
    windowToOpenGLMatrix.scale(2.0f / width(), (flipY ? -2.0f : 2.0f) / height());

    if (includeMVP)
        return (getMVPMatrix() * windowToOpenGLMatrix);
    else
        return windowToOpenGLMatrix;
}

QMatrix4x4 AxialSliceWidget::getNIFTIToOpenGLMatrix(bool includeMVP, bool flipY) const
{
    QMatrix4x4 NIFTIToOpenGLMatrix;

    NIFTIToOpenGLMatrix.translate(-1.0f, flipY ? 1.0f : -1.0f);
    NIFTIToOpenGLMatrix.scale(2.0f / fatImage->getXDim(), (flipY ? -2.0f : 2.0f) / fatImage->getYDim());

    if (includeMVP)
        return (getMVPMatrix() * NIFTIToOpenGLMatrix);
    else
        return NIFTIToOpenGLMatrix;
}


void AxialSliceWidget::setUndoStack(QUndoStack *stack)
{
    undoStack = stack;
}

void AxialSliceWidget::resetView()
{
    // Reset translation and scaling factors
    translation = QVector3D(0.0f, 0.0f, 0.0f);
    scaling = 1.0f;

    // Update the screen
    updateCrosshairLine();
    update();
}

void AxialSliceWidget::initializeGL()
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
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/axialslice.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/axialslice.frag");
    program->link();
    program->bind();

    program->setUniformValue("tex", 0);

    initializeSliceView();
    initializeCrosshairLine();
    initializeColorMaps();
}

void AxialSliceWidget::initializeSliceView()
{
    // get context opengl-version
    qDebug() << "----------------- AxialSliceWidget ---------------------------";
    qDebug() << "Widget OpenGL: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid();
    qDebug() << "Really used OpenGL: " << context()->format().majorVersion() << "." << context()->format().minorVersion();
    qDebug() << "OpenGL information: VENDOR:       " << (const char*)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    qDebug() << "";

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
    glGenTextures(1, &this->slicePrimTexture);
    glGenTextures(1, &this->sliceSecdTexture);

    // Release (unbind) all
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void AxialSliceWidget::initializeCrosshairLine()
{
    lineStart = QPoint(0, 0);
    lineEnd = QPoint(0, 0);
    lineWidth = 0;
}

void AxialSliceWidget::initializeColorMaps()
{
    // Width of the 1D texture map. The larger the number the more points in the 1D texture and the smoother the colors
    const int width = 256;
    // Internal format is how the data will be stored in the GPU. Format/type is how the data is represented
    const GLint internalFormat = GL_RGBA32F;
    const GLenum format = GL_RGBA;
    const GLenum type = GL_UNSIGNED_BYTE;

    // Create textures for each of the color maps
    glGenTextures((GLsizei)ColorMap::Count, &this->colorMapTexture[0]);

    for (int i = 0; i < (int)ColorMap::Count; ++i)
    {
        QPixmap pixmap;

        if (!pixmap.load(colorMapImageName[i]))
        {
            qWarning() << "Unable to load color map number " << i << " located at " << colorMapImageName[i];
            continue;
        }

        // For simplicity, just convert the image to a 32-bit value. That way we know what the format is
        QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGBA8888);

        if (image.height() != 1)
        {
            qWarning() << "Height must be 1 for color map number " << i << " located at " << colorMapImageName[i] << ": " << image.height();
            continue;
        }

        // This is a formula to determine if a number is a power of two easily. If equal to zero, it is a power of two
        if ((image.width() & (image.width() - 1)) != 0)
        {
            qWarning() << "Width must be power of two for color map number " << i << " located at " << colorMapImageName[i] << ": " << image.width();
            continue;
        }

        // Bind the texture and setup the parameters for it
        glBindTexture(GL_TEXTURE_1D, colorMapTexture[i]);
        // These parameters say that the color value for a pixel will be chosen based on the nearest pixel value. This creates a more blocky effect
        // since it will not be linearly interpolated like GL_LINEAR
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // This parameter will clamp points to [0.0, 1.0]. This means that anything above 1.0 will become 1.0
        // and anything below 0.0 will become 0.0
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);

        glTexImage1D(GL_TEXTURE_1D, 0, internalFormat, image.width(), 0, format, type, image.bits());
    }
}

void AxialSliceWidget::updateTexture()
{
    cv::Mat primMatrix;
    cv::Mat secdMatrix;
    switch (displayType)
    {
        case SliceDisplayType::FatOnly:
        {
            // Get the slice for the fat image. If the result is empty then there was an error retrieving the slice
            primMatrix = fatImage->getAxialSlice(location.z(), true);
            if (primMatrix.empty())
            {
                qDebug() << "Unable to retrieve axial slice " << location.z() << " from the fat image. Matrix returned empty.";
                return;
            }

            // The normalize function does quite a bit here. It converts the matrix to a 32-bit float and normalizes it
            // between 0.0f to 1.0f based on the min/max value. This does not affect the original 3D matrix in fatImage
            cv::normalize(primMatrix.clone(), primMatrix, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);
        }
        break;

        case SliceDisplayType::WaterOnly:
        {
            // Get the slice for the water image. If the result is empty then there was an error retrieving the slice
            primMatrix = waterImage->getAxialSlice(location.z(), true);
            if (primMatrix.empty())
            {
                qDebug() << "Unable to retrieve axial slice " << location.z() << " from the water image. Matrix returned empty.";
                return;
            }

            // The normalize function does quite a bit here. It converts the matrix to a 32-bit float and normalizes it
            // between 0.0f to 1.0f based on the min/max value. This does not affect the original 3D matrix in waterImage
            cv::normalize(primMatrix.clone(), primMatrix, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);
        }
        break;

        case SliceDisplayType::FatFraction:
        {
            // Get the slice for the fat/water image. If the result is empty then there was an error retrieving the slice
            cv::Mat fatTemp = fatImage->getAxialSlice(location.z(), true);
            cv::Mat waterTemp = waterImage->getAxialSlice(location.z(), true);
            if (fatTemp.empty() || waterTemp.empty())
            {
                qDebug() << "Unable to retrieve axial slice " << location.z() << " from the fat or water image. Matrix returned empty.";
                return;
            }

            // The normalize function does quite a bit here. It converts the matrix to a 32-bit float and normalizes it
            // between 0.0f to 1.0f based on the min/max value. This does not affect the original 3D matrix in fatImage/waterImage
            cv::normalize(fatTemp.clone(), fatTemp, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);
            cv::normalize(waterTemp.clone(), waterTemp, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);

            primMatrix = fatTemp / (fatTemp + waterTemp);
        }
        break;

        case SliceDisplayType::WaterFraction:
        {
            // Get the slice for the fat/water image. If the result is empty then there was an error retrieving the slice
            cv::Mat fatTemp = fatImage->getAxialSlice(location.z(), true);
            cv::Mat waterTemp = waterImage->getAxialSlice(location.z(), true);
            if (fatTemp.empty() || waterTemp.empty())
            {
                qDebug() << "Unable to retrieve axial slice " << location.z() << " from the fat or water image. Matrix returned empty.";
                return;
            }

            // The normalize function does quite a bit here. It converts the matrix to a 32-bit float and normalizes it
            // between 0.0f to 1.0f based on the min/max value. This does not affect the original 3D matrix in fatImage/waterImage
            cv::normalize(fatTemp.clone(), fatTemp, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);
            cv::normalize(waterTemp.clone(), waterTemp, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);

            primMatrix = waterTemp / (fatTemp + waterTemp);
        }
        break;

        case SliceDisplayType::FatWater:
        {
            // Get the slice for the fat image. If the result is empty then there was an error retrieving the slice
            primMatrix = fatImage->getAxialSlice(location.z(), true);
            if (primMatrix.empty())
            {
                qDebug() << "Unable to retrieve axial slice " << location.z() << " from the fat image. Matrix returned empty.";
                return;
            }

            // The normalize function does quite a bit here. It converts the matrix to a 32-bit float and normalizes it
            // between 0.0f to 1.0f based on the min/max value. This does not affect the original 3D matrix in fatImage
            cv::normalize(primMatrix.clone(), primMatrix, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);

            // Get the slice for the water image. If the result is empty then there was an error retrieving the slice
            // The secondary matrix is the water image in this case
            secdMatrix = waterImage->getAxialSlice(location.z(), true);
            if (secdMatrix.empty())
            {
                qDebug() << "Unable to retrieve axial slice " << location.z() << " from the water image. Matrix returned empty.";
                return;
            }

            // The normalize function does quite a bit here. It converts the matrix to a 32-bit float and normalizes it
            // between 0.0f to 1.0f based on the min/max value. This does not affect the original 3D matrix in waterImage
            cv::normalize(secdMatrix.clone(), secdMatrix, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);
        }
        break;

        case SliceDisplayType::WaterFat:
        {
            // Get the slice for the fat image. If the result is empty then there was an error retrieving the slice
            primMatrix = waterImage->getAxialSlice(location.z(), true);
            if (primMatrix.empty())
            {
                qDebug() << "Unable to retrieve axial slice " << location.z() << " from the water image. Matrix returned empty.";
                return;
            }

            // The normalize function does quite a bit here. It converts the matrix to a 32-bit float and normalizes it
            // between 0.0f to 1.0f based on the min/max value. This does not affect the original 3D matrix in fatImage
            cv::normalize(primMatrix.clone(), primMatrix, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);

            // Get the slice for the water image. If the result is empty then there was an error retrieving the slice
            // The secondary matrix is the water image in this case
            secdMatrix = fatImage->getAxialSlice(location.z(), true);
            if (secdMatrix.empty())
            {
                qDebug() << "Unable to retrieve axial slice " << location.z() << " from the fat image. Matrix returned empty.";
                return;
            }

            // The normalize function does quite a bit here. It converts the matrix to a 32-bit float and normalizes it
            // between 0.0f to 1.0f based on the min/max value. This does not affect the original 3D matrix in waterImage
            cv::normalize(secdMatrix.clone(), secdMatrix, 0.0f, 1.0f, cv::NORM_MINMAX, CV_32FC1);
        }
        break;
    }

    // Bind the texture and setup the parameters for it
    glBindTexture(GL_TEXTURE_2D, slicePrimTexture);
    // These parameters basically say the pixel value is equal to the average of the nearby pixel values when magnifying or minifying the values
    // Essentially, when stretching or shrinking the texture to the screen, it will smooth out the pixel values instead of making it look blocky
    // like GL_NEAREST would.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Get the OpenGL datatype of the matrix
    auto dataType = NumericType::OpenCV(primMatrix.type());
    // Upload the texture data from the matrix to the texture. The internal format is 32 bit floats with one channel for red
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, fatImage->getXDim(), fatImage->getYDim(), 0, dataType->openGLFormat, dataType->openGLType, primMatrix.data);

    // Repeat the process if the second matrix is available
    if (!secdMatrix.empty())
    {
        glBindTexture(GL_TEXTURE_2D, sliceSecdTexture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        dataType = NumericType::OpenCV(primMatrix.type());

        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, fatImage->getXDim(), fatImage->getYDim(), 0, dataType->openGLFormat, dataType->openGLType, secdMatrix.data);
    }

    // If there was an error, then say something
    GLenum err;
    if ((err = glGetError()) != GL_NO_ERROR)
        qDebug() << "Unable to upload texture image for axial slice " << location.z() << ". Error code: " << err;

    update();
}

void AxialSliceWidget::updateCrosshairLine()
{
    // Start with calculating the thickness of each axial layer according to the translation/scaling factors
    // Need to get this in terms of window coordinates b/c that is the system QPainter uses

    // The NIFTI to Window matrix will be used to simply convert from NIFTI coordinates to window coordinates
    QMatrix4x4 NIFTIToWindowMatrix = getWindowToNIFTIMatrix().inverted();

    // Create a start point at top of screen (coronalSlice 0)
    // Set the end point to be one slice down (coronalsSlice 1)
    QVector4D start(0.0f, 0.0f, 0.0f, 1.0f);
    QVector4D end(0.0f, 1.0f, 0.0f, 1.0f);

    // Transform the start and end points from OpenGL to Window matrix (this takes into account the MVP matrix)
    start = NIFTIToWindowMatrix * start;
    end = NIFTIToWindowMatrix * end;

    // Get the difference between the start and end point. The length is the necessary line width to accurately show how
    // large one coronal slice is
    QVector4D delta = end - start;

    // Set lineWidth to be an integer value of the delta length. However, the
    // lineWidth must be at least 1 so that the pen will be shown
    lineWidth = std::max((int)std::floor(delta.length()), 1);

    // Start line is left of screen at specified y value and end value is right of screen at specified y value
    // Note: These are in NIFTI coordinate system (0 -> XDim - 1, 0 -> YDim - 1)
    start = QVector4D(0, location.y(), 0.0f, 1.0f);
    end = QVector4D(fatImage->getXDim() - 1, location.y(), 0.0f, 1.0f);

    // Transform the points to window matrix (factors in OpenGL MVP)
    start = NIFTIToWindowMatrix * start;
    end = NIFTIToWindowMatrix * end;

    // Convert to 2D points
    lineStart = start.toPoint();
    lineEnd = end.toPoint();

    update();
}

void AxialSliceWidget::resizeGL(int w, int h)
{
    // Shuts compiler up about unused variables w and h.
    (void)w;
    (void)h;

    // Do nothing if fat/water images are not loaded
    if (!isLoaded())
        return;

    updateCrosshairLine();
}

void AxialSliceWidget::paintGL()
{
    // Do nothing if fat/water images are not loaded
    if (!isLoaded())
        return;

    QPainter painter(this);

    // With painter, call beginNativePainting before doing any custom OpenGL commands
    painter.beginNativePainting();

    // Sets up transparency for the primary and secondary textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Disable depth testing with blending setup
    glDisable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT);

    // Calculate the ModelViewProjection (MVP) matrix to transform the location of the axial slices
    QMatrix4x4 mvpMatrix = getMVPMatrix();

    program->bind();
    program->setUniformValue("brightness", brightness);
    program->setUniformValue("contrast", contrast);
    program->setUniformValue("MVP", mvpMatrix);

    program->setUniformValue("opacity", primOpacity);

    // Bind the VAO, bind texture to GL_TEXTURE0, bind VBO, bind IBO
    glBindVertexArray(sliceVertexObject);
    glBindBuffer(GL_ARRAY_BUFFER, sliceVertexBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sliceIndexBuf);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, slicePrimTexture);
    glBindTexture(GL_TEXTURE_1D, colorMapTexture[(int)primColorMap]);

    // Draw a triangle strip of 4 elements which is two triangles. The indices are unsigned shorts
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);

    if (displayType == SliceDisplayType::FatWater || displayType == SliceDisplayType::WaterFat)
    {
        program->setUniformValue("opacity", secdOpacity);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sliceSecdTexture);
        glBindTexture(GL_TEXTURE_1D, colorMapTexture[(int)secdColorMap]);

        // Draw a triangle strip of 4 elements which is two triangles. The indices are unsigned shorts
        // Drawing again for the secondary image
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
    }

    // Release (unbind) the binded objects in reverse order
    // This is a simple protocol to prevent anything happening to the objects outside of this function without
    // explicitly binding the objects
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_1D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    program->release();

    painter.endNativePainting();

    // Draw crosshair line
    painter.setPen(QPen(Qt::red, lineWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(lineStart, lineEnd);

    painter.setTransform(getWindowToNIFTIMatrix().inverted().toTransform());

    auto axialPoints = points[location.z()];
    for (int i = 0; i < (int)TracingLayer::Count; ++i)
    {
        auto layer = axialPoints[i];
        if (tracingLayerVisible[i] && layer.size() > 0)
        {
            painter.setPen(QPen(tracingLayerColors[i], 1, Qt::SolidLine, Qt::SquareCap));
            painter.drawPoints(layer.data(), (int)layer.size());
        }
    }
}

void AxialSliceWidget::addPoint(QPointF mouseCoord)
{
    QPoint NIFTICoord = (getWindowToNIFTIMatrix() * mouseCoord).toPoint();

    if (QRect(0, 0, fatImage->getXDim(), fatImage->getYDim()).contains(NIFTICoord))
    {
        auto &layerPoints = points[location.z()][(int)tracingLayer];

        // Mouse Command Created is a boolean variable to store whether a TracingPointsAddCommand
        // was added since the mouse has been clicked down. This is necessary because the user can
        // mouse down outside of the NIFTI image, move around, and end up outside the NIFTI image
        // without drawing a single point. Therefore, a boolean is checked and a command is created
        // whenever the first point is to be added to the list.
        if (!mouseCommand)
        {
            mouseCommand = new TracingPointsAddCommand((int)layerPoints.size(), this);
            undoStack->push(mouseCommand);
        }

        if ((layerPoints.size() - mouseCommand->getIndex()) > 0) // There must be existing points in the list
        {
            // Perform linear interpolation between current point (NIFTICoord)
            // and last point in layerPoints
            const QPointF lastPoint = layerPoints.back();

            // Dont add duplicate points
            if (lastPoint == NIFTICoord)
                return;

            // The length approximates how many units are between the two points
            // Therefore, 1/length should be the approximate step value in percent
            // to get all of the points between currentPoint and lastPoint
            const float steps = 1 / QVector2D(NIFTICoord - lastPoint).length();
            float percent = 0.0f;

            while (percent < 1.0f)
            {
                QPoint interPoint = util::lerp(lastPoint, NIFTICoord, percent).toPoint();

                // Skip if the previous point equals this point
                // This will occur when rounding errors happen and return the same point
                if (interPoint != layerPoints.back())
                    layerPoints.push_back(interPoint);

                percent += steps;
            }
        }

        layerPoints.push_back(NIFTICoord);

        const int smoothPoints = std::min(2, (int)layerPoints.size() - mouseCommand->getIndex());
        if (smoothPoints > 2)
        {
            const float a = 0.25f;
            const size_t stopLayerPoints = layerPoints.size() - 1 - smoothPoints;
            for (size_t i = layerPoints.size() - 2; i > stopLayerPoints; i--)
            {
                const QPointF pEnd = layerPoints[i] * a + layerPoints[i+1] * (1 - a);
                layerPoints[i] = pEnd.toPoint();
            }
        }

        update();
    }
}

void AxialSliceWidget::mouseMoveEvent(QMouseEvent *eventMove)
{
    if (startDraw)
    {
        QPointF mouseCoord = eventMove->pos();
        addPoint(mouseCoord);
    }
    else if (startPan)
    {
        // Change in mouse x/y based on last mouse position
        QPointF curMousePos = eventMove->pos();
        QPointF lastMousePos_ = lastMousePos;

        // Get matrix for converting from window to OpenGL coordinate system
        // Note: Do not apply MVP because we do not want to see movement based on
        // scaling (this means dont flip it either)
        QMatrix4x4 windowToOpenGLMatrix = getWindowToOpenGLMatrix(false, false);

        // Apply transformation to current and last mouse position
        curMousePos = windowToOpenGLMatrix * curMousePos;
        lastMousePos_ = windowToOpenGLMatrix * lastMousePos_;

        // Get the delta
        QPointF delta = (curMousePos - lastMousePos_);

        // Push a new move command on the undoStack. This will call the command but also keep track
        // of it if an undo or redo action is called. redo function is called immediately.
        undoStack->push(new AxialMoveCommand(delta, this, moveID));

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
            startPan = false;

        startDraw = true;

        addPoint(eventPress->pos());
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
        moveID = (((int)moveID + 1) > (int)CommandID::AxialMoveEnd) ? CommandID::AxialMove : (CommandID)((int)moveID + 1);
        lastMousePos = eventPress->pos();
    }
}

void AxialSliceWidget::mouseReleaseEvent(QMouseEvent *eventRelease)
{
    if (eventRelease->button() == Qt::LeftButton && startDraw)
    {
        addPoint(eventRelease->pos());

        startDraw = false;
        mouseCommand = NULL;
    }
    else if (eventRelease->button() == Qt::MiddleButton && startPan)
    {
        startPan = false;
    }
}

void AxialSliceWidget::wheelEvent(QWheelEvent *event)
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
        undoStack->push(new AxialScaleCommand(scaleDelta, this));
    }
}

AxialSliceWidget::~AxialSliceWidget()
{
    // Destroy the VAO, VBO, and IBO
    glDeleteVertexArrays(1, &sliceVertexObject);
    glDeleteBuffers(1, &sliceVertexBuf);
    glDeleteBuffers(1, &sliceIndexBuf);
    glDeleteTextures(1, &slicePrimTexture);
    glDeleteTextures(1, &sliceSecdTexture);
    glDeleteTextures((int)ColorMap::Count, &colorMapTexture[0]);
    delete program;
}
