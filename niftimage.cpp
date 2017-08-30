#include "niftimage.h"

NIFTImage::NIFTImage() : upper(NULL), lower(NULL), subConfig(NULL), xDim(0), yDim(0), zDim(0)
{

}

NIFTImage::NIFTImage(nifti_image *upper, nifti_image *lower, SubjectConfig *config) : NIFTImage()
{
    setImage(upper, lower, config);
}

NIFTImage::NIFTImage(SubjectConfig *config) : NIFTImage()
{
    setSubjectConfig(config);
}

/* setImage sets the upper and lower NIFTI images for the class.
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
bool NIFTImage::setImage(nifti_image *upper, nifti_image *lower, SubjectConfig *config)
{
    // If one of the parameters given is NULL, then return false
    if (!upper || !lower)
    {
        qDebug() << "Null upper/lower image pointer was given. Unable to set the NIFTI image";
        return false;
    }

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

    if (config)
    {
        this->subConfig = config;
    }
    else if (!subConfig)
    {
        qDebug() << "No subject configuration data for NIFTI image. Unable to load NIFTI image without data.";
        return false;
    }

    // Define the number of slices that will be transferred for the upper/lower images
    // This is NOT the same as the size of the upper/lower NIFTI images because only a
    // certain portion is extracted from the upper/lower images
    int upperLength = subConfig->imageUpperSuperior - subConfig->imageUpperInferior + 1;
    int lowerLength = subConfig->imageLowerSuperior - subConfig->imageLowerInferior + 1;

    // xDim, yDim, and zDim are the dimensions of the resulting image with upper and lower portions put together.
    // The xDim and yDim stay the same as the two images but the zDim is upper slices plus the number of lower slices
    xDim = upper->dim[1];
    yDim = upper->dim[2];
    zDim = upperLength + lowerLength;

    // Create matrix of zDim x yDim x xDim.
    // The default datatype of the matrix is to match the NIFTI file datatype
    auto numericType = NumericType::NIFTI(upper->datatype);
    int datatype = CV_MAKETYPE(numericType->openCVTypeNoChannel, 1);
    data = cv::Mat({zDim, yDim, xDim}, datatype, cv::Scalar(0));

    /* Upper */
    cv::Mat upperROI = data({cv::Range(lowerLength, zDim), cv::Range::all(), cv::Range::all()});

    cv::Mat upperMat({upper->dim[3], upper->dim[2], upper->dim[1]}, datatype, upper->data);

    // Read orientation field for the NIFTI image. If the orientation field is not RAS (+X -> Right, +Y -> Anterior, +Z -> Superior),
    // then correct it by flipping the image
    orientImage(upper, upperMat);

    // Copy imageUpperInferior to imageUpperSuperior to upperMatROI
    cv::Mat upperMatROI = upperMat({cv::Range(subConfig->imageUpperInferior, subConfig->imageUpperSuperior + 1), cv::Range::all(), cv::Range::all()});

    upperMatROI.copyTo(upperROI);

    /* Lower */
    cv::Mat lowerROI = data({cv::Range(0, lowerLength), cv::Range::all(), cv::Range::all()});

    cv::Mat lowerMat({lower->dim[3], lower->dim[2], lower->dim[1]}, datatype, lower->data);

    // Read orientation field for the NIFTI image. If the orientation field is not RAS (+X -> Right, +Y -> Anterior, +Z -> Superior),
    // then correct it by flipping the image
    orientImage(lower, lowerMat);

    // Copy imageLowerInferior to imageLowerSuperior to lowerMatROI
    cv::Mat lowerMatROI = lowerMat({cv::Range(subConfig->imageLowerInferior, subConfig->imageLowerSuperior + 1), cv::Range::all(), cv::Range::all()});

    lowerMatROI.copyTo(lowerROI);

    // Flip the matrix once it is loaded
    // Flip along Z-axis and Y-axis
//    cv::Mat dataFlipped;

//    opencv::flip(data, dataFlipped, 0);
//    opencv::flip(dataFlipped, data, 1);

    return true;
}

// Orient NIFTI image so the coordinate system is RAS
void NIFTImage::orientImage(nifti_image *image, cv::Mat &mat)
{
    // Read orientation field for the NIFTI image. If the orientation field is not RAS (+X -> Right, +Y -> Anterior, +Z -> Superior),
    // then correct it by flipping the image
    int xOrienCode, yOrienCode, zOrienCode;
    nifti_mat44_to_orientation(image->sto_xyz, &xOrienCode, &yOrienCode, &zOrienCode);

    // If +X -> L (R2L), then flip by Z-axis
    if (xOrienCode == NIFTI_R2L)
    {
        cv::Mat dataFlipped;

        opencv::flip(mat, dataFlipped, 2);
        mat = dataFlipped;
    }

    // If +Y -> P (A2P), then flip by X-axis
    if (yOrienCode == NIFTI_A2P)
    {
        cv::Mat dataFlipped;

        opencv::flip(mat, dataFlipped, 0);
        mat = dataFlipped;
    }

    // If +Z -> I (S2I), then flip by Y-axis
    if (zOrienCode == NIFTI_S2I)
    {
        cv::Mat dataFlipped;

        opencv::flip(mat, dataFlipped, 1);
        mat = dataFlipped;
    }
}

/* setSubjectConfig sets the configuration data for the subject.
 * It is used for stitching together the upper/lower NIFTI images into one coherent 3D image.
 *
 * Returns:
 *      true - No error occurred and the operation was successful
 *      false - An error occurred
 */
bool NIFTImage::setSubjectConfig(SubjectConfig *config)
{
    if (!config)
    {
        qDebug() << "Null config pointer was given. Unable to set the subject configuration in NIFTI class";
        return false;
    }

    this->subConfig = config;
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
bool NIFTImage::checkImage() const
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
        // Note: The pixel dimensions are disabled because Subject 3 initial had different dimensions slightly
        //upper->pixdim[1] != lower->pixdim[1] ||
        //upper->pixdim[2] != lower->pixdim[2] ||
        //upper->pixdim[3] != lower->pixdim[3] ||
        upper->xyz_units != lower->xyz_units ||
        upper->datatype != lower->datatype ||
        upper->nbyper != lower->nbyper)
    {
        qDebug() << "Dimensions are not compatible between upper and lower image. Upper is the first value printed out and lower is the second value\n\n"
                 << "dim[0]: " << upper->dim[0] << " " << lower->dim[0] << "\n"
                 << "dim[1]: " << upper->dim[1] << " " << lower->dim[1] << "\n"
                 << "dim[2]: " << upper->dim[2] << " " << lower->dim[2] << "\n"
                 << "pixdim[1]: " << upper->pixdim[1] << " " << lower->pixdim[1] << "\n"
                 << "pixdim[2]: " << upper->pixdim[2] << " " << lower->pixdim[2] << "\n"
                 << "pixdim[3]: " << upper->pixdim[3] << " " << lower->pixdim[3] << "\n"
                 << "xyz_units: " << upper->xyz_units << " " << lower->xyz_units << "\n"
                 << "datatype: " << upper->datatype << " " << lower->datatype << "\n"
                 << "nbyper: " << upper->nbyper << " " << lower->nbyper << "\n";
        return false;
    }

    // If there is not a valid OpenGL/OpenCV datatype for the upper image, then return false.
    // Note: The upper and lower have the same datatypes since the above if statement is true
    auto type = NumericType::NIFTI(upper->datatype);
    if (type == NULL || type->openGLFormat == (GLenum)DataType::None || type->openGLType == (GLenum)DataType::None || type->openCVType == (int)DataType::None)
    {
        qDebug() << "Unsupported NIFTI datatype in OpenGL: " << upper->datatype;
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
bool NIFTImage::compatible(NIFTImage *image) const
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
                 << "pixdim[1]: " << upper->pixdim[1] << " " << upperOther->pixdim[1] << "\n"
                 << "pixdim[2]: " << upper->pixdim[2] << " " << upperOther->pixdim[2] << "\n"
                 << "pixdim[3]: " << upper->pixdim[3] << " " << upperOther->pixdim[3] << "\n"
                 << "xyz_units: " << upper->xyz_units << " " << upperOther->xyz_units << "\n"
                 << "datatype: " << upper->datatype << " " << upperOther->datatype << "\n"
                 << "nbyper: " << upper->nbyper << " " << upperOther->nbyper << "\n";
        return false;
    }

    return true;
}

nifti_image *NIFTImage::getUpperImage() const
{
    return upper;
}

nifti_image *NIFTImage::getLowerImage() const
{
    return lower;
}

int NIFTImage::getXDim() const
{
    return xDim;
}

int NIFTImage::getYDim() const
{
    return yDim;
}

int NIFTImage::getZDim() const
{
    return zDim;
}

bool NIFTImage::isLoaded() const
{
    return (upper && lower);
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

/* getAxialSlice is similar to getRegion function but instead returns one axial slice of the data matrix.
 *
 * Returns:
 *      cv::Mat - Matrix of the slice. If an error occurred, an empty matrix is returned.
 */
cv::Mat NIFTImage::getAxialSlice(int z, bool clone)
{
    if (data.empty() || z < 0 || z >= zDim)
        return cv::Mat();

    const cv::Range region[] = { cv::Range(z, z + 1), cv::Range::all(), cv::Range::all() };

    cv::Mat ret;

    if (clone)
        ret = cv::Mat(data, region).clone();
    else
        ret = cv::Mat(data, region);

    int dims[] = { data.size[1], data.size[2] };
    return ret.reshape(0, 2, dims);
}

/* getCoronalSlice is similar to getRegion function but instead returns one coronal slice of the data matrix.
 *
 * Returns:
 *      cv::Mat - Matrix of the slice. If an error occurred, an empty matrix is returned.
 */
cv::Mat NIFTImage::getCoronalSlice(int y, bool clone)
{
    if (data.empty() || y < 0 || y >= yDim)
        return cv::Mat();

    const cv::Range region[] = { cv::Range::all(), cv::Range(y, y + 1), cv::Range::all() };

    cv::Mat ret;

    if (clone)
        ret = cv::Mat(data, region).clone();
    else
        ret = cv::Mat(data, region);

    int dims[] = { data.size[0], data.size[2] };
    return ret.reshape(0, 2, dims);
}

/* getSaggitalSlice is similar to getRegion function but instead returns one saggital slice of the data matrix.
 *
 * Returns:
 *      cv::Mat - Matrix of the slice. If an error occurred, an empty matrix is returned.
 */
cv::Mat NIFTImage::getSaggitalSlice(int x, bool clone)
{
    if (data.empty() || x < 0 || x >= xDim)
        return cv::Mat();

    const cv::Range region[] = { cv::Range::all(), cv::Range::all(), cv::Range(x, x + 1) };

    cv::Mat ret;

    if (clone)
        ret = cv::Mat(data, region).clone();
    else
        ret = cv::Mat(data, region);

    int dims[] = { data.size[0], data.size[1] };
    return ret.reshape(0, 2, dims);
}

/* getType returns a NumericType pointer that stores information about the valid data types
 * supported across the various libraries included. Some examples include OpenGL and OpenCV
 *
 * Returns:
 *      NumericType * - NULL if data matrix is empty or invalid data type in data matrix is used
 *                      otherwise, a valid NumericType is returned
 */
const NumericType *NIFTImage::getType() const
{
    if (data.empty())
        return NULL;

    return NumericType::OpenCV(data.type());
}

NIFTImage::~NIFTImage()
{
    if (upper && lower)
    {
        nifti_image_free(upper);
        nifti_image_free(lower);
    }
}
