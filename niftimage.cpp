#include "niftimage.h"

NIFTImage::NIFTImage()
{
    // Initialize values to defaults
    this->upper = NULL;
    this->lower = NULL;

    this->xDim = 0;
    this->yDim = 0;
    this->zDim = 0;
}

NIFTImage::NIFTImage(nifti_image *upper, nifti_image *lower)
{
    // Initialize values to defaults
    this->upper = NULL;
    this->lower = NULL;

    this->xDim = 0;
    this->yDim = 0;
    this->zDim = 0;

    // Set the image upper and lower
    setImage(upper, lower);
}

/* setImage does exactly what it says. It sets the upper and lower NIFTI images for the class.
 * In addition, the old upper and lower NIFTI images are deleted and a new data matrix is created.
 * The new data matrix is of size zDim x yDim x xDim where xDim and yDim are dim[1] and dim[2] of the
 * NIFTI file and zDim is the sum of dim[3] of the upper and lower image.
 *
 * Afterwards, the data matrix is filled with the upper and lower image data. Effectively, the upper
 * and lower images are stitched into one large matrix. The datatype of the resulting matrix is the same
 * as the NIFTI images.
 *
 * Returns:
 *      true - No error occurred and the operation was successful
 *      false - An error occurred
 */
bool NIFTImage::setImage(nifti_image *upper, nifti_image *lower)
{
    // If one of the parameters given is NULL, then return false
    if (!upper || !lower)
        return false;

    // If there are previous upper and lower images, free the nifti files
    if (this->upper && this->lower)
    {
        nifti_image_free(this->upper);
        nifti_image_free(this->lower);
    }

    // Set the upper and lower images to the ones given
    this->upper = upper;
    this->lower = lower;

    // Check the parameters in the NIFTI file and make sure they are compatible
    // For example, the dimensions in X/Y direciton must match for the upper and lower to be able to stitch them together
    if (!checkImage())
        return false;

    // xDim, yDim, and zDim are the dimensions of the resulting image with upper and lower portions put together.
    // The xDim and yDim stay the same as the two images but the zDim is upper slices plus the number of lower slices
    xDim = upper->dim[1];
    yDim = upper->dim[2];
    zDim = upper->dim[3] + lower->dim[3];

    // Create matrix of zDim x yDim x xDim.
    // The default datatype of the matrix is to match the NIFTI file datatype
    int dims[] = { zDim, yDim, xDim };
    int datatype = niftiToOpenCVDatatype(upper);
    data = cv::Mat(3, dims, datatype, cv::Scalar(0));

    // Now that the empty matrix, data, is created it is time to fill in the upper and lower image data
    // upperROI is the region in the data matrix where the data will go for the upper image
    const cv::Range upperRegion[] = { cv::Range(0, upper->dim[3]), cv::Range::all(), cv::Range::all() };
    cv::Mat upperROI = data(upperRegion);

    // Create a matrix denoted upperMat of the NIFTI file data for upper image
    int dimsUpper[] = { upper->dim[3], upper->dim[2], upper->dim[1] };
    cv::Mat upperMat(3, dimsUpper, datatype, upper->data);
    // Copy the upperMat matrix to the upperROI. Note: This will affect the data matrix
    upperMat.copyTo(upperROI);

    // lowerROI is the region in the data matrix where the data will go for the lower image
    const cv::Range lowerRegion[] = { cv::Range(upper->dim[3], zDim), cv::Range::all(), cv::Range::all() };
    cv::Mat lowerROI = data(lowerRegion);

    // Create a matrix denoted lowerMat of the NIFTI file data for lower image
    int dimsLower[] = { lower->dim[3], lower->dim[2], lower->dim[1] };
    cv::Mat lowerMat(3, dimsLower, datatype, lower->data);
    // Copy the lowerMat matrix to the lowerROI. Note: This will affect the data matrix
    lowerMat.copyTo(lowerROI);

    return true;
}

/* checkImage determines the compatibility of the upper and lower image formats within the class.
 * Compatibility is defined as the ability to stitch the upper and lower image formats into the same
 * matrix. A number of things are checked in the NIFTI file format of the upper and lower images.
 *
 * Returns:
 *      true - Upper/lower images are compatible
 *      false - Upper/lower images are not compatible or an error occurred
 */
bool NIFTImage::checkImage()
{
    // If upper or lower are not set, then there is nothing to check.
    if (!upper || !lower)
    {
        qDebug() << "Upper or lower image not set in checkImage function";
        return false;
    }

    // This checks that the upper and lower have equivalent dimensions, pixel dimensions, units,
    // and datatypes. If not, false is returned because the images are not compatible with each other
    if (upper->dim[0] != 3 || lower->dim[0] != 3 ||
        upper->dim[1] != lower->dim[1] ||
        upper->dim[2] != lower->dim[2] ||
        upper->pixdim[1] != lower->pixdim[1] ||
        upper->pixdim[2] != lower->pixdim[2] ||
        upper->pixdim[3] != lower->pixdim[3] ||
        upper->xyz_units != lower->xyz_units ||
        upper->datatype != lower->datatype ||
        upper->nbyper != lower->nbyper)
    {
        qDebug() << "Dimensions are not compatible between upper and lower image. Upper is the first value printed out and lower is the second value\n\n"
                 << "dim[0]: " << upper->dim[0] << " " << lower->dim[0] << "\n"
                 << "dim[1]: " << upper->dim[1] << " " << lower->dim[1] << "\n"
                 << "dim[2]: " << upper->dim[2] << " " << lower->dim[2] << "\n"
                 << "pixdim[1]: " << upper->dim[1] << " " << lower->dim[1] << "\n"
                 << "pixdim[2]: " << upper->dim[2] << " " << lower->dim[2] << "\n"
                 << "pixdim[3]: " << upper->dim[3] << " " << lower->dim[3] << "\n"
                 << "xyz_units: " << upper->xyz_units << " " << lower->xyz_units << "\n"
                 << "datatype: " << upper->datatype << " " << lower->datatype << "\n"
                 << "nbyper: " << upper->nbyper << " " << lower->nbyper << "\n";
        return false;
    }

    // If there is not a valid OpenGL datatype for the upper image, then return false.
    // Note: The upper and lower have the same datatypes since the above if statement is true
    if (niftiToOpenGLDatatype(upper) == NULL)
    {
        qDebug() << "Unsupported NIFTI datatype in OpenGL: " << upper->datatype;
        return false;
    }

    // If there is not a valid OpenCV datatype for the upper image, then return false.
    // Note: The upper and lower have the same datatypes since the above if statement is true
    if (niftiToOpenCVDatatype(upper) == -1)
    {
        qDebug() << "Unsupported NIFTI datatype in OpenCV: " << upper->datatype;
        return false;
    }

    return true;
}

/* compatible checks the compatibility of one NIFTIImage class to another. It is very similar to the
 * checkImage function except that it is checking between another NIFTImage class. One primary difference
 * is that this checks that the Z dimension of the two images are equal while the checkImage class does not
 * check that because they are stitched together regardless
 *
 * Returns:
 *      true - This class is compatible with the given image class
 *      false - Not compatible or an error occurred
 */
bool NIFTImage::compatible(NIFTImage *image)
{
    // Get the NIFTI image of the other class
    nifti_image *upperOther = image->getUpperImage();

    // If the upper or lower of this class are not set, then we cannot
    // check compatibility
    if (!upper || !lower)
    {
        qDebug() << "Upper or lower image not set in compatible function";
        return false;
    }

    // The number of dimensions must be equal to 3 on this class and the other class.
    // The X, Y, and Z dimensions of this class and the other class must be equal.
    // The pixel dimensions, units, and datatype of the structures must also be equal.
    // If one of these are not satisfied, then return false
    if (upper->dim[0] != 3 || upperOther->dim[0] != 3 ||
        upper->dim[1] != upperOther->dim[1] ||
        upper->dim[2] != upperOther->dim[2] ||
        upper->dim[3] != upperOther->dim[3] ||
        upper->pixdim[1] != upperOther->pixdim[1] ||
        upper->pixdim[2] != upperOther->pixdim[2] ||
        upper->pixdim[3] != upperOther->pixdim[3] ||
        upper->xyz_units != upperOther->xyz_units ||
        upper->datatype != upperOther->datatype ||
        upper->nbyper != upperOther->nbyper)
    {
        qDebug() << "Dimensions are not compatible between water and fat image.\n\n"
                 << "dim[0]: " << upper->dim[0] << " " << upperOther->dim[0] << "\n"
                 << "dim[1]: " << upper->dim[1] << " " << upperOther->dim[1] << "\n"
                 << "dim[2]: " << upper->dim[2] << " " << upperOther->dim[2] << "\n"
                 << "dim[3]: " << upper->dim[3] << " " << upperOther->dim[3] << "\n"
                 << "pixdim[1]: " << upper->dim[1] << " " << upperOther->dim[1] << "\n"
                 << "pixdim[2]: " << upper->dim[2] << " " << upperOther->dim[2] << "\n"
                 << "pixdim[3]: " << upper->dim[3] << " " << upperOther->dim[3] << "\n"
                 << "xyz_units: " << upper->xyz_units << " " << upperOther->xyz_units << "\n"
                 << "datatype: " << upper->datatype << " " << upperOther->datatype << "\n"
                 << "nbyper: " << upper->nbyper << " " << upperOther->nbyper << "\n";
        return false;
    }

    return true;
}

nifti_image *NIFTImage::getUpperImage()
{
    return upper;
}

nifti_image *NIFTImage::getLowerImage()
{
    return lower;
}

int NIFTImage::getXDim()
{
    return xDim;
}

int NIFTImage::getYDim()
{
    return yDim;
}

int NIFTImage::getZDim()
{
    return zDim;
}

/* getRegion returns a region of the data matrix. The region vector can be done using initializer
 * lists in C++11 which makes this a simple function to use. The number of items in the region
 * vector must be 3 because the dimension of the data matrix is 3. The clone parameter is whether
 * the matrix region extracted is to be cloned. Cloning the region will mean any changes made to it
 * will not affect data matrix. If it is not cloned, then any changes to the region will be seen in
 * the data matrix.
 *
 * Note: The order of the range's in the vector does matter. It should be ordered like this: {z, y, x}
 *
 * Returns:
 *      cv::Mat - Matrix for the region. If an error occurred, an empty matrix is returned.
 */
cv::Mat NIFTImage::getRegion(std::vector<cv::Range> region, bool clone)
{
    if (data.empty() || region.size() != 3)
        return cv::Mat();

    if (clone)
        return cv::Mat(data, region.data()).clone();
    else
        return cv::Mat(data, region.data());
}

/* getSlice is similar to getRegion function but instead returns one slice of the data matrix.
 *
 * Returns:
 *      cv::Mat - Matrix of the slice. If an error occurred, an empty matrix is returned.
 */
cv::Mat NIFTImage::getSlice(int z, bool clone)
{
    if (data.empty() || z < 0 || z > zDim - 1)
        return cv::Mat();

    const cv::Range region[] = { cv::Range(z, z + 1), cv::Range::all(), cv::Range::all() };

    cv::Mat ret;

    if (clone)
        ret = cv::Mat(data, region).clone();
    else
        ret = cv::Mat(data, region);

    int dims[] = { data.size[2], data.size[1] };
    return ret.reshape(0, 2, dims);
}

/*void NIFTImage::normalize(int datatype, int scale, int offset)
{
    if (data.empty())
        return;

    double minValue, maxValue;
    cv::minMaxIdx(data, &minValue, &maxValue);
    qDebug() << "MinValue: " << minValue << " MaxValue: " << maxValue;

    cv::Mat temp;

    data.convertTo(temp, CV_32FC1);

    cv::Mat normalizedData = offset + (temp / 579.0f) * scale;
    data = normalizedData;
    //cv::normalize(data, data, 0, 65000, cv::NORM_MINMAX, CV_16UC1);


    QFile file("C:/Users/addis/Desktop/matrix.txt");
    file.open(QIODevice::WriteOnly);
        //return 0;

    QTextStream stream(&file);

    for (int z = 0; z < 1; ++z)
    {
        for (int y = 0; y < yDim; ++y)
        {
            for (int x = 0; x < xDim; ++x)
            {
                //stream << data.at<float>(z, y, x) << " ";
            }
            stream << "\n";
        }
        stream << "-------------------\n\n\n--------------------------------\n";
    }

    file.close();

    //data = offset + (data / 100.0f);
    //normalizedData.convertTo(data, datatype);
}*/

/* getOpenGLDatatype returns the equivalent OpenGL datatype necessary for the data
 * matrix! Note: The NIFTI file datatype is irrelevant because the data matrix is the
 * ultimate source of data. The NIFTI file format is only used initially to import the
 * data and the matrix could be converted to another format.
 *
 * Returns:
 *      GLenum * -
 *              GLenum *[0] - OpenCV datatype of the data matrix
 *              GLenum *[1] - OpenGL channels needed such as GL_RED, GL_RGBA, GL_RGB, etc
 *              GLenum *[2] - OpenGL datatype such as GL_UNSIGNED_SHORT, GL_BYTE, etc
 *              Returns NULL pointer if the datatype is not supported in OpenGL
 */
GLenum *NIFTImage::getOpenGLDatatype()
{
    if (data.empty())
        return NULL;

    return openCVToOpenGLDatatype(data.type());
}

/* niftiToOpenGLDatatype converts a NIFTI datatype into a OpenGL datatype.
 * This is done by simply using a lookup table to map between NIFTI datatypes
 * to OpenGL datatypes.
 *
 * Returns:
 *      GLenum * -
 *              GLenum *[0] - NIFTI datatype
 *              GLenum *[1] - OpenGL channels needed such as GL_RED, GL_RGBA, GL_RGB, etc
 *              GLenum *[2] - OpenGL datatype such as GL_UNSIGNED_SHORT, GL_BYTE, etc
 *              Returns NULL pointer if the datatype is not supported in OpenGL
 */
GLenum *NIFTImage::niftiToOpenGLDatatype(nifti_image *image)
{
    for (int i = 0; niftiToOpenGLLUT[i] != NULL; ++i)
    {
        if (niftiToOpenGLLUT[i][0] == image->datatype)
            return niftiToOpenGLLUT[i];
    }

    return NULL;
}

/* openCVToOpenGLDatatype converts an OpenCV datatype into a OpenGL datatype.
 * This is done by simply using a lookup table to map between OpenCV datatypes
 * to OpenGL datatypes.
 *
 * Returns:
 *      GLenum * -
 *              GLenum *[0] - OpenCV datatype
 *              GLenum *[1] - OpenGL channels needed such as GL_RED, GL_RGBA, GL_RGB, etc
 *              GLenum *[2] - OpenGL datatype such as GL_UNSIGNED_SHORT, GL_BYTE, etc
 *              Returns NULL pointer if the datatype is not supported in OpenGL
 */
GLenum *NIFTImage::openCVToOpenGLDatatype(int datatype)
{
    for (int i = 0; OpenCVToOpenGLLUT[i] != NULL; ++i)
    {
        if (OpenCVToOpenGLLUT[i][0] == datatype)
            return OpenCVToOpenGLLUT[i];
    }

    return NULL;
}

/* niftiToOpenCVDatatype converts a NIFTI datatype into a OpenCV datatype.
 * This is done by simply using a lookup table to map between NIFTI datatypes
 * to OpenCV datatypes.
 *
 * Returns:
 *      int - OpenCV datatype with necessary channels such as CV_16UC1, CV_16SC1, etc
 *            Returns -1 if the datatype is not supported in OpenCV
 */
int NIFTImage::niftiToOpenCVDatatype(nifti_image *image)
{
    for (int i = 0; niftiToOpenCVLUT[i] != NULL; ++i)
    {
        if (niftiToOpenCVLUT[i][0] == image->datatype)
            return niftiToOpenCVLUT[i][1];
    }

    return -1;
}

NIFTImage::~NIFTImage()
{
    if (upper && lower)
    {
        nifti_image_free(upper);
        nifti_image_free(lower);
    }
}
