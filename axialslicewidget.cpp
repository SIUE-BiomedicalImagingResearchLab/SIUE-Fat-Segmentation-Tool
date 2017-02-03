#include "axialslicewidget.h"
#include "commands.h"

AxialSliceWidget::AxialSliceWidget(QWidget *parent) : QOpenGLWidget(parent),
    displayType(SliceDisplayType::FatOnly), fatImage(NULL), waterImage(NULL), tracingData(NULL),
    tracingLayerColors({ Qt::blue, Qt::darkCyan, Qt::cyan, Qt::magenta, Qt::yellow, Qt::green }), mouseCommand(NULL),
    slicePrimTexture(NULL), sliceSecdTexture(NULL),
    location(0, 0, 0, 0), locationLabel(NULL), primColorMap(ColorMap::Gray), primOpacity(1.0f), secdColorMap(ColorMap::Gray), secdOpacity(1.0f),
    brightness(0.0f), contrast(1.0f), tracingLayer(TracingLayer::EAT), drawMode(DrawMode::Points),
    startDraw(false), startPan(false), moveID(CommandID::AxialMove)
{
    this->tracingLayerVisible.fill(true);
    this->traceTextureInit.fill(false);
}

void AxialSliceWidget::setup(NIFTImage *fat, NIFTImage *water, TracingData *tracing)
{
    if (!fat || !water || !tracing)
    {
        qDebug() << "Invalid fat image, water image or tracing data in setup: " << fat << water << tracing;
        return;
    }

    fatImage = fat;
    waterImage = water;
    tracingData = tracing;

    location = QVector4D(0, 0, 0, 0);
}

bool AxialSliceWidget::isLoaded() const
{
    return (fatImage->isLoaded() && waterImage->isLoaded());
}

void AxialSliceWidget::imageLoaded()
{
    sliceTexturePrimInit = false;
    sliceTextureSecdInit = false;
    this->traceTextureInit.fill(false);

    dirty |= Dirty::Slice | Dirty::TracesAll | Dirty::Crosshair;
    update();
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
        dirty |= Dirty::Crosshair;

    // If Z value changed, then update the texture
    if (delta.z())
        dirty |= (Dirty::Slice | Dirty::TracesAll);

    // Update location label
    if (locationLabel)
        locationLabel->setText(QObject::tr("Location: (%1, %2, %3)").arg(this->location.x()).arg(this->location.y()).arg(this->location.z()));

    update();
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
        qWarning() << "Invalid display type was specified for AxialSliceWidget: " << (int)type;
        return;
    }

    displayType = type;

    // This will recreate the texture because the display type has changed
    dirty |= Dirty::Slice;
    update();
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
        qWarning() << "Invalid primary color map was specified for AxialSliceWidget: " << (int)map;
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
        qWarning() << "Invalid primary opacity level was specified for AxialSliceWidget: " << opacity;
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
        qWarning() << "Invalid secondary color map was specified for AxialSliceWidget: " << (int)map;
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
        qWarning() << "Invalid secondary opacity level was specified for AxialSliceWidget: " << opacity;
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
        qWarning() << "Invalid brightness was specified for AxialSliceWidget: " << brightness;
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
        qWarning() << "Invalid contrast was specified for AxialSliceWidget: " << contrast;
        return;
    }

    this->contrast = contrast;

    // Redraw the screen because the contrast has changed
    update();
}

DrawMode AxialSliceWidget::getDrawMode() const
{
    return drawMode;
}

void AxialSliceWidget::setDrawMode(DrawMode mode)
{
    drawMode = mode;
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
        qWarning() << "Invalid current tracing layer was specified for AxialSliceWidget: " << (int)layer;
        return;
    }

    tracingLayer = layer;
}

bool AxialSliceWidget::getTracingLayerVisible(TracingLayer layer) const
{
    // If the layer is out of the acceptable range, then do nothing
    if (layer < TracingLayer::EAT || layer >= TracingLayer::Count)
    {
        qWarning() << "Invalid current tracing layer was specified for AxialSliceWidget: " << (int)layer;
        return false;
    }

    return tracingLayerVisible[(int)layer];
}

void AxialSliceWidget::setTracingLayerVisible(TracingLayer layer, bool value)
{
    // If the layer is out of the acceptable range, then do nothing
    if (layer < TracingLayer::EAT || layer >= TracingLayer::Count)
    {
        qWarning() << "Invalid current tracing layer was specified for AxialSliceWidget: " << (int)layer;
        return;
    }

    tracingLayerVisible[(int)layer] = value;
}

TracingLayerData &AxialSliceWidget::getTraceSlices(TracingLayer layer)
{
    if (layer == TracingLayer::Count)
        layer = tracingLayer;

    return (*tracingData)[layer];
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
    const QString timeDir = "times";
    QString layerFullPath[(int)TracingLayer::Count];
    QString layerTimeFullPath[(int)TracingLayer::Count];

    // Create absolute path to each of the layer filenames by joining directory path (path) with each filename (layerFilename)
    for (int i = 0; i < (int)TracingLayer::Count; ++i)
    {
        layerFullPath[i] = QDir(path).filePath(layerFilename[i]);
        layerTimeFullPath[i] = QDir(path).filePath(timeDir + "/" + layerFilename[i]); // TODO: Better solution? Not platform independent
    }

    if (promptOnOverwrite)
    {
        for (QString fullPath : layerFullPath)
        {
            if (QFileInfo(fullPath).exists())
            {
                // Not a fan of the prompt option
                if (QMessageBox::warning((QWidget *)parent(), "Confirm Save As", "Tracing data already exists in folder.\nDo you want to replace it?", QMessageBox::Yes, QMessageBox::No)
                        != QMessageBox::Yes)
                    return false;
                break;
            }
        }
    }

    // Create timing directory if it does not already exist
    QDir timesDir = QDir(path).filePath(timeDir);
    if (!timesDir.exists())
        timesDir.mkdir(".");

    for (int i = 0; i < (int)TracingLayer::Count; ++i)
    {
        QFile sliceFile(layerFullPath[i]);
        if (!sliceFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            qWarning() << "Error opening file to save tracing data. Skipping layer: " << layerFullPath[i];
            continue;
        }

        QTextStream sliceStream(&sliceFile);
        sliceStream << fatImage->getZDim() << endl;

        QFile timeFile(layerTimeFullPath[i]);
        if (!timeFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            qWarning() << "Error opening file to save tracing data. Skipping layer: " << layerTimeFullPath[i];
            continue;
        }

        QTextStream timeStream(&timeFile);
        timeStream << fatImage->getZDim() << endl;

        const int zDim = fatImage->getZDim();
        for (int z = 0; z < zDim; ++z)
        {
            auto &traceLayer = (*tracingData)[i];
            cv::Mat slice = traceLayer.getAxialSlice(z);

            cv::Mat points;
            opencv::findNonZero(slice, points);

            // Only sort if there are points to sort
            if (points.total() > 0)
            {
                // Sort based on Z, then Y, then X value.
                std::sort(points.begin<cv::Vec2i>(), points.end<cv::Vec2i>(), [](const cv::Vec2i &a, const cv::Vec2i &b) {
                    return !((a[0] >= b[0]) && (a[0] != b[0] || a[1] >= b[1]));
                });
            }

            sliceStream << "#" << z << endl;
            sliceStream << points.total() << endl;

            for (int i = 0; i < points.total(); ++i)
            {
                const cv::Vec2i point = points.at<cv::Vec2i>(i);
                sliceStream << forcepoint << (float)point[1] << " " << (float)point[0] << " " << (float)z << endl;
            }

            auto time = traceLayer.time[z];
            auto h = std::chrono::duration_cast<std::chrono::hours>(time);
            auto m = std::chrono::duration_cast<std::chrono::minutes>(time -= h);
            auto s = std::chrono::duration_cast<std::chrono::seconds>(time -= m);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time -= s);
            timeStream << "#" << z << " " << h.count() << "h " << m.count() << "m " << s.count() << "s " << ms.count() << "ms" << endl;
        }
    }

    // Set stack to clean to notify the application that no unsaved changes are present
    undoStack->setClean();

    return true;
}


bool AxialSliceWidget::loadTracingData(QString path)
{
    if (!isLoaded())
    {
        qWarning() << "Tracing data cannot be imported into an application until the correct NIFTI image is loaded first.\nPlease load the correct NIFTI file and then try again.";
        return false;
    }

    bool hasData = false;
    for (auto &layer : this->tracingData->layers)
    {
        if (cv::countNonZero(layer.data) > 0)
        {
            hasData = true;
            break;
        }
    }

    // If there is data in the fat layers and the stack is not clean, then prompt user if they are sure they want to import tracing data
    // NOTE: This has the flaw that even simple settings such as changing color map and stuff will make the stack not clean.
    if (hasData && !undoStack->isClean())
    {
        if (QMessageBox::warning((QWidget *)parent(), "Confirm Import", "Unsaved tracing data is present in this image.\nAre you sure you want to load this new tracing data and discard current changes?", QMessageBox::Yes, QMessageBox::No)
                != QMessageBox::Yes)
            return false;
    }

    const QString layerFilename[(int)TracingLayer::Count] = {"EAT.txt", "IMAT.txt", "PAAT.txt", "PAT.txt", "SCAT.txt", "VAT.txt"};
    const QString timeDir = "times";
    QString layerFullPath[(int)TracingLayer::Count];
    QString layerTimeFullPath[(int)TracingLayer::Count];

    // Create absolute path to each of the layer filenames by joining directory path (path) with each filename (layerFilename)
    for (int i = 0; i < (int)TracingLayer::Count; ++i)
    {
        layerFullPath[i] = QDir(path).filePath(layerFilename[i]);
        layerTimeFullPath[i] = QDir(path).filePath(timeDir + "/" + layerFilename[i]); // TODO: Better solution? Not platform independent
    }

    for (int i = 0; i < (int)TracingLayer::Count; ++i)
    {
        QFile sliceFile(layerFullPath[i]);
        if (!sliceFile.open(QIODevice::ReadOnly))
        {
            qWarning() << "Error opening file to load tracing data. Skipping layer: " << layerFullPath[i];
            continue;
        }

        QTextStream sliceStream(&sliceFile);

        QFile timeFile(layerTimeFullPath[i]);
        if (!timeFile.open(QIODevice::ReadOnly))
        {
            qWarning() << "Error opening file to load tracing data. Skipping layer: " << layerTimeFullPath[i];
            continue;
        }

        QTextStream timeStream(&timeFile);

        int zDim, zDim_;
        sliceStream >> zDim;
        timeStream >> zDim_;


        if (zDim != fatImage->getZDim() || zDim_ != fatImage->getZDim())
        {
            qWarning() << "Number of axial slices in the data does not match the NIFTI image loaded.";
            return false; // Note: Return false because the other layers should be mismatched as well
        }

        auto &layer = (*tracingData)[i];

        // Discard previous data by setting everything to 0
        layer.data.setTo(0);

        for (int z = 0; z < zDim; ++z)
        {
            // Skip the #Z where Z is the axial slice
            sliceStream.skipWhiteSpace();
            sliceStream.readLine();
            timeStream.skipWhiteSpace();

            // Get the number of points on the slices
            int numPoints = 0;
            sliceStream >> numPoints;

            float x, y, z_;
            for (int ii = 0; ii < numPoints; ++ii)
            {
                sliceStream >> x >> y >> z_;

                if ((z < 0 || z >= fatImage->getZDim()) || (y < 0 || y >= fatImage->getYDim()) || (x < 0 || x >= fatImage->getXDim()))
                {
                    qWarning() << "A point specified was outside the boundary of the current NIFTI image: (" << z << "," << y << "," << x << ")";
                    return false;
                }

                layer.set(x, y, z);
            }

            // Read timing
            char dummy;
            int z__;
            QString str;
            unsigned int h, m, s, ms;
            timeStream >> dummy >> z__ >> ws >> h >> str >> ws >> m >> str >> ws >> s >> str >> ws >> ms >> str >> ws;
            layer.time[z] = (std::chrono::hours(h) + std::chrono::minutes(m) + std::chrono::seconds(s) + std::chrono::milliseconds(ms));
        }
    }

    // Clear the undoStack so that all of the tracing commands are deleted from beforehand
    // This may cause unwanted commands to be deleted but it is okay
    undoStack->clear();

    // Update all traces textures and update screen
    dirty |= Dirty::TracesAll;
    update();

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

void AxialSliceWidget::setDirty(int bit)
{
    dirty |= bit;
}

void AxialSliceWidget::resetView()
{
    // Reset translation and scaling factors
    translation = QVector3D(0.0f, 0.0f, 0.0f);
    scaling = 1.0f;

    // Update the screen
    dirty |= Dirty::Crosshair;
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

    sliceProgram = new QOpenGLShaderProgram();
    sliceProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/axialslice.vert");
    sliceProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/axialslice.frag");
    sliceProgram->link();
    sliceProgram->bind();
    if (!sliceProgram->log().isEmpty())
        qDebug() << "Slice Program Log: " << sliceProgram->log();
    glCheckError();

    sliceProgram->setUniformValue("tex", 0);
    sliceProgram->setUniformValue("mappingTexture", 0);

    traceProgram = new QOpenGLShaderProgram();
    traceProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/fattraces.vert");
    traceProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fattraces.frag");
    traceProgram->link();
    traceProgram->bind();
    if (!traceProgram->log().isEmpty())
        qDebug() << "Trace Program Log: " << traceProgram->log();
    glCheckError();

    traceProgram->setUniformValue("tex", 0);

    initializeSliceView();
    initializeTracing();
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

    sliceTexturePrimInit = false;
    sliceTextureSecdInit = false;

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
    glCheckError();

    // Generate index buffer for the axial slice. The sliceIndices data is uploaded to the IBO
    glGenBuffers(1, &sliceIndexBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sliceIndexBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sliceIndices.size() * sizeof(GLushort), sliceIndices.constData(), GL_STATIC_DRAW);
    glCheckError();

    // Generate VAO for the axial slice vertices uploaded. Location 0 is the position and location 1 is the texture position
    glGenVertexArrays(1, &sliceVertexObject);
    glBindVertexArray(sliceVertexObject);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, VertexPT::PosTupleSize, GL_FLOAT, true, VertexPT::stride(), static_cast<const char *>(0) + VertexPT::posOffset());
    glVertexAttribPointer(1, VertexPT::TexPosTupleSize, GL_FLOAT, true, VertexPT::stride(), static_cast<const char *>(0) + VertexPT::texPosOffset());
    glCheckError();

    // Generate a blank texture for the axial slice
    glGenTextures(1, &this->slicePrimTexture);
    glGenTextures(1, &this->sliceSecdTexture);
    glCheckError();

    // Release (unbind) all
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void AxialSliceWidget::initializeTracing()
{
    this->traceTextureInit.fill(false);

    // Setup the trace vertices
    traceVertices.clear();
    traceVertices.append(VertexPT(QVector3D(-1.0f, -1.0f, 0.0f), QVector2D(0.0f, 0.0f)));
    traceVertices.append(VertexPT(QVector3D(-1.0f, 1.0f, 0.0f), QVector2D(0.0f, 1.0f)));
    traceVertices.append(VertexPT(QVector3D(1.0f, -1.0f, 0.0f), QVector2D(1.0f, 0.0f)));
    traceVertices.append(VertexPT(QVector3D(1.0f, 1.0f, 0.0f), QVector2D(1.0f, 1.0f)));

    // Setup the trace indices
    traceIndices.clear();
    traceIndices.append({ 0, 1, 2, 3});

    // Generate vertex buffer for the axial slice. The sliceVertices data is uploaded to the VBO
    glGenBuffers(1, &traceVertexBuf);
    glBindBuffer(GL_ARRAY_BUFFER, traceVertexBuf);
    glBufferData(GL_ARRAY_BUFFER, traceVertices.size() * sizeof(VertexPT), traceVertices.constData(), GL_STATIC_DRAW);
    glCheckError();

    // Generate index buffer for the axial slice. The sliceIndices data is uploaded to the IBO
    glGenBuffers(1, &traceIndexBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, traceIndexBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, traceIndices.size() * sizeof(GLushort), traceIndices.constData(), GL_STATIC_DRAW);
    glCheckError();

    // Generate VAO for the axial slice vertices uploaded. Location 0 is the position and location 1 is the texture position
    glGenVertexArrays(1, &traceVertexObject);
    glBindVertexArray(traceVertexObject);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, VertexPT::PosTupleSize, GL_FLOAT, true, VertexPT::stride(), static_cast<const char *>(0) + VertexPT::posOffset());
    glVertexAttribPointer(1, VertexPT::TexPosTupleSize, GL_FLOAT, true, VertexPT::stride(), static_cast<const char *>(0) + VertexPT::texPosOffset());
    glCheckError();

    // Generate a blank texture for the axial slice
    glGenTextures((int)TracingLayer::Count, &this->traceTextures[0]);
    glCheckError();

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
    glCheckError();

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
        glCheckError();

        glTexImage1D(GL_TEXTURE_1D, 0, internalFormat, image.width(), 0, format, type, image.bits());
        glCheckError();
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
                qWarning() << "Unable to retrieve axial slice " << location.z() << " from the fat image. Matrix returned empty.";
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
                qWarning() << "Unable to retrieve axial slice " << location.z() << " from the water image. Matrix returned empty.";
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
                qWarning() << "Unable to retrieve axial slice " << location.z() << " from the fat or water image. Matrix returned empty.";
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
                qWarning() << "Unable to retrieve axial slice " << location.z() << " from the fat or water image. Matrix returned empty.";
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
                qWarning() << "Unable to retrieve axial slice " << location.z() << " from the fat image. Matrix returned empty.";
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
                qWarning() << "Unable to retrieve axial slice " << location.z() << " from the water image. Matrix returned empty.";
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
                qWarning() << "Unable to retrieve axial slice " << location.z() << " from the water image. Matrix returned empty.";
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
                qWarning() << "Unable to retrieve axial slice " << location.z() << " from the fat image. Matrix returned empty.";
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
    glCheckError();

    // Get the OpenGL datatype of the matrix
    auto dataType = NumericType::OpenCV(primMatrix.type());
    // Upload the texture data from the matrix to the texture. The internal format is 32 bit floats with one channel for red
    // If it hasnt been initialized yet or needs to be reinitialized to a different size, use glTexImage2D, otherwise use
    // the quicker method glTexSubImage2D which just overwrites old data
    if (!sliceTexturePrimInit)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, fatImage->getXDim(), fatImage->getYDim(), 0, dataType->openGLFormat, dataType->openGLType, primMatrix.data);
        sliceTexturePrimInit = true;
    }
    else
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fatImage->getXDim(), fatImage->getYDim(), dataType->openGLFormat, dataType->openGLType, primMatrix.data);

    glCheckError();

    // Repeat the process if the second matrix is available
    if (!secdMatrix.empty())
    {
        glBindTexture(GL_TEXTURE_2D, sliceSecdTexture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glCheckError();

        dataType = NumericType::OpenCV(primMatrix.type());

        // If it hasnt been initialized yet or needs to be reinitialized to a different size, use glTexImage2D, otherwise use
        // the quicker method glTexSubImage2D which just overwrites old data
        if (!sliceTextureSecdInit)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, fatImage->getXDim(), fatImage->getYDim(), 0, dataType->openGLFormat, dataType->openGLType, secdMatrix.data);
            sliceTextureSecdInit = true;
        }
        else
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fatImage->getXDim(), fatImage->getYDim(), dataType->openGLFormat, dataType->openGLType, secdMatrix.data);

        glCheckError();
    }

    dirty &= ~Dirty::Slice;
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

    dirty &= ~Dirty::Crosshair;
}

void AxialSliceWidget::updateTrace(TracingLayer layer)
{
    // Bind the texture and setup the parameters for it
    glBindTexture(GL_TEXTURE_2D, traceTextures[(int)layer]);
    // Set pixel parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glCheckError();

    // Grab current slice of the tracing data matrix at given layer
    cv::Mat matrix = (*tracingData)[layer].getAxialSlice(location.z());

    // Get the OpenGL datatype of the matrix
    auto dataType = NumericType::OpenCV(matrix.type());

    // Upload the texture data from the matrix to the texture. The internal format is an 8 bit char with one channel for red
    // If it hasnt been initialized yet or needs to be reinitialized to a different size, use glTexImage2D, otherwise use
    // the quicker method glTexSubImage2D which just overwrites old data
    if (!traceTextureInit[(int)layer])
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, fatImage->getXDim(), fatImage->getYDim(), 0, dataType->openGLFormat, dataType->openGLType, matrix.data);
        traceTextureInit[(int)layer] = true;
    }
    else
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, fatImage->getXDim(), fatImage->getYDim(), dataType->openGLFormat, dataType->openGLType, matrix.data);

    glCheckError();
}

void AxialSliceWidget::updateTraces(bool allOrCurrent)
{
    if (allOrCurrent)
    {
        for (int i = 0; i < (int)TracingLayer::Count; ++i)
            updateTrace((TracingLayer)i);
        dirty &= ~(Dirty::TracesAll | Dirty::Traces);
    }
    else
    {
        updateTrace(tracingLayer);
        dirty &= ~Dirty::Traces;
    }
}

void AxialSliceWidget::resizeGL(int w, int h)
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

void AxialSliceWidget::paintGL()
{
    // Do nothing if fat/water images are not loaded
    if (!isLoaded())
        return;

    // Update relevant OpenGL objects if dirty
    if (dirty & Dirty::Slice)
        updateTexture();

    if (dirty & Dirty::Crosshair)
        updateCrosshairLine();

    if (dirty & Dirty::Traces)
        updateTraces(false);

    if (dirty & Dirty::TracesAll)
        updateTraces(true);

    // After updating, begin rendering
    QPainter painter(this);

    // With painter, call beginNativePainting before doing any custom OpenGL commands
    painter.beginNativePainting();

    // Sets up transparency for the primary and secondary textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Disable depth testing with blending setup
    glDisable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT);
    glCheckError();

    // Calculate the ModelViewProjection (MVP) matrix to transform the location of the axial slices
    QMatrix4x4 mvpMatrix = getMVPMatrix();

    sliceProgram->bind();
    sliceProgram->setUniformValue("brightness", brightness);
    sliceProgram->setUniformValue("contrast", contrast);
    sliceProgram->setUniformValue("MVP", mvpMatrix);
    glCheckError();

    sliceProgram->setUniformValue("opacity", primOpacity);
    glCheckError();

    // Bind the VAO, bind texture to GL_TEXTURE0, bind VBO, bind IBO
    glBindVertexArray(sliceVertexObject);
    glBindBuffer(GL_ARRAY_BUFFER, sliceVertexBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sliceIndexBuf);
    glCheckError();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, slicePrimTexture);
    glBindTexture(GL_TEXTURE_1D, colorMapTexture[(int)primColorMap]);
    glCheckError();

    // Draw a triangle strip of 4 elements which is two triangles. The indices are unsigned shorts
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
    glCheckError();

    if (displayType == SliceDisplayType::FatWater || displayType == SliceDisplayType::WaterFat)
    {
        sliceProgram->setUniformValue("opacity", secdOpacity);
        glCheckError();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sliceSecdTexture);
        glBindTexture(GL_TEXTURE_1D, colorMapTexture[(int)secdColorMap]);
        glCheckError();

        // Draw a triangle strip of 4 elements which is two triangles. The indices are unsigned shorts
        // Drawing again for the secondary image
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
        glCheckError();
    }

    // Release (unbind) the binded objects in reverse order
    // This is a simple protocol to prevent anything happening to the objects outside of this function without
    // explicitly binding the objects
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_1D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    sliceProgram->release();

    painter.endNativePainting();

    // Draw crosshair line
    painter.setPen(QPen(Qt::red, lineWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(lineStart, lineEnd);

    // With painter, call beginNativePainting before doing any custom OpenGL commands
    // This is called again because the crosshair line needs to be after the NIFT image but before the tracing
    // This shows the user if they have placed a trace over the crosshair line
    painter.beginNativePainting();

    // Sets up transparency for the tracing colors; NOTE: The tracing colors can be set to a transparent value
    // if desired
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    traceProgram->bind();
    traceProgram->setUniformValue("MVP", mvpMatrix);
    glCheckError();

    // Bind the VAO, bind texture to GL_TEXTURE0, bind VBO, bind IBO
    glBindVertexArray(traceVertexObject);
    glBindBuffer(GL_ARRAY_BUFFER, traceVertexBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, traceIndexBuf);
    glCheckError();

    for (int i = 0; i < (int)TracingLayer::Count; ++i)
    {
        if (tracingLayerVisible[i])
        {
            // Tell the shader program what color to use for this layer
            traceProgram->setUniformValue("traceColor", tracingLayerColors[i]);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, traceTextures[i]);
            glCheckError();

            // Draw a triangle strip of 4 elements which is two triangles. The indices are unsigned shorts
            glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
            glCheckError();
        }
    }

    // Release (unbind) the binded objects in reverse order
    // This is a simple protocol to prevent anything happening to the objects outside of this function without
    // explicitly binding the objects
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_1D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    traceProgram->release();

    painter.endNativePainting();
}

void AxialSliceWidget::addPoint(QPointF mouseCoord)
{
    QPoint NIFTICoord = (getWindowToNIFTIMatrix() * mouseCoord).toPoint();

    if (QRect(0, 0, fatImage->getXDim(), fatImage->getYDim()).contains(NIFTICoord))
    {
        // Mouse Command Created is a boolean variable to store whether a TracingPointsAddCommand
        // was added since the mouse has been clicked down. This is necessary because the user can
        // mouse down outside of the NIFTI image, move around, and end up outside the NIFTI image
        // without drawing a single point. Therefore, a boolean is checked and a command is created
        // whenever the first point is to be added to the list.
        if (!mouseCommand)
        {
            mouseCommand = new TracingPointsAddCommand(NIFTICoord, this);
            undoStack->push(mouseCommand);
        }
        else
        {
            mouseCommand->addPoint(NIFTICoord);
        }
    }
}

void AxialSliceWidget::mouseMoveEvent(QMouseEvent *eventMove)
{
    if (!isLoaded())
        return;

    if (startDraw)
    {
        addPoint(eventMove->pos());
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
    if (!isLoaded())
        return;

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
        drawTimer.start();
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
    if (!isLoaded())
        return;

    if (eventRelease->button() == Qt::LeftButton && startDraw)
    {
        addPoint(eventRelease->pos());
        (*tracingData)[tracingLayer].time[location.z()] += std::chrono::milliseconds(drawTimer.elapsed());
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
    glDeleteVertexArrays(1, &traceVertexObject);
    glDeleteBuffers(1, &traceVertexBuf);
    glDeleteBuffers(1, &traceIndexBuf);
    glDeleteTextures((int)TracingLayer::Count, &traceTextures[0]);
    glDeleteTextures((int)ColorMap::Count, &colorMapTexture[0]);
    delete sliceProgram;
    delete traceProgram;
}
