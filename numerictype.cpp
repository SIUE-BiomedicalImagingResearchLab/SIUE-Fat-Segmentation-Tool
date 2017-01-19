#include "numerictype.h"

static const int numericTypeLUTSize = 18;
static const NumericType numericTypeLUT[numericTypeLUTSize] =
{
    { DataType::UnsignedChar,   GL_RED,     GL_UNSIGNED_BYTE,   CV_8UC1,    CV_8U,      DT_UINT8 },
    { DataType::Char,           GL_RED,     GL_BYTE,            CV_8SC1,    CV_8S,      DT_INT8 },
    { DataType::UnsignedChar,   GL_RG,      GL_UNSIGNED_BYTE,   CV_8UC2,    CV_8U,      DT_UINT8 },
    { DataType::Char,           GL_RG,      GL_BYTE,            CV_8SC2,    CV_8S,      DT_INT8 },
    { DataType::UnsignedShort,  GL_RED,     GL_UNSIGNED_SHORT,  CV_16UC1,   CV_16U,     DT_UINT16 },
    { DataType::Short,          GL_RED,     GL_SHORT,           CV_16SC1,   CV_16S,     DT_INT16 },
    { DataType::UnsignedShort,  GL_RG,      GL_UNSIGNED_SHORT,  CV_16UC2,   CV_16U,     DT_UINT16 },
    { DataType::Short,          GL_RG,      GL_SHORT,           CV_16SC2,   CV_16S,     DT_INT16 },
    { DataType::UnsignedInt,    GL_RED,     GL_UNSIGNED_INT,    CV_32SC1,   CV_32S,     DT_UINT8 }, // OpenCV does not have data type for unsigned int: use signed int
    { DataType::Int,            GL_RED,     GL_INT,             CV_32SC1,   CV_32S,     DT_INT8 },
    { DataType::UnsignedInt,    GL_RG,      GL_UNSIGNED_INT,    CV_32SC2,   CV_32S,     DT_UINT32 }, // OpenCV does not have data type for unsigned int: use signed int
    { DataType::Int,            GL_RG,      GL_INT,             CV_32SC2,   CV_32S,     DT_INT32 },
    { DataType::Float,          GL_RED,     GL_FLOAT,           CV_32FC1,   CV_32F,     DT_FLOAT32 },
    { DataType::Float,          GL_RG,      GL_FLOAT,           CV_32FC2,   CV_32F,     DT_FLOAT32 },
    { DataType::UnsignedChar,   GL_RGB,     GL_UNSIGNED_BYTE,   CV_8UC3,    CV_8U,      DT_RGB24 },
    { DataType::Char,           GL_RGBA,    GL_UNSIGNED_BYTE,   CV_8UC4,    CV_8U,      DT_RGBA32 },
    { DataType::Double,         GL_RED,     GL_DOUBLE,          CV_64FC1,   CV_64F,     DT_FLOAT64 },
    { DataType::Double,         GL_RG,      GL_DOUBLE,          CV_64FC2,   CV_64F,     DT_FLOAT64 }
};

/* Okay, so there are times where you do not know the type of a data structure and want to get the maximum value for it.
 * For example, you may load data from a NIFTI file or even an image with OpenCV. There are quite a few options that it
 * can be. However, let's say you just want to initialize it or replace part of it with the maximum value of that data
 * type. This would essentially make it white in a certain location. This cannot be done without a huge switch statement
 * for the various data types. Furthermore, it doesn't help you out because the cv::Scalar option just converts it to a
 * double anyways. Therefore, the min/max values are stored as doubles until C++ offers better support for this (if it
 * ever does; it may be a limitation of the compiler). No trunucation should occur so it will be alright.
 * MSVC still complains about narrowing conversion from char -> double, even though this is not the case? I just disable the
 * warning and reenable after done.
 */
#ifdef _MSC_VER
#pragma warning(disable:4838)
#endif // _MSC_VER
static const double minMaxLUT[][2] =
{
    {std::numeric_limits<bool>::min(), std::numeric_limits<bool>::max()},
    {std::numeric_limits<unsigned char>::min(), std::numeric_limits<unsigned char>::max()},
    {std::numeric_limits<char>::min(), std::numeric_limits<char>::max()},
    {std::numeric_limits<unsigned short>::min(), std::numeric_limits<unsigned short>::max()},
    {std::numeric_limits<short>::min(), std::numeric_limits<short>::max()},
    {std::numeric_limits<unsigned int>::min(), std::numeric_limits<unsigned int>::max()},
    {std::numeric_limits<int>::min(), std::numeric_limits<int>::max()},
    {std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max()},
    {std::numeric_limits<long>::min(), std::numeric_limits<long>::max()},
    {std::numeric_limits<float>::min(), std::numeric_limits<float>::max()},
    {std::numeric_limits<double>::min(), std::numeric_limits<double>::max()}
};
#ifdef _MSC_VER
#pragma warning(default:4838)
#endif // _MSC_VER

NumericType::NumericType(DataType type_, GLenum openGLFormat_, GLenum openGLType_, int openCVType_, int openCVTypeNoChannel_, int NIFTIType_) :
    type(type_)
#ifndef NUMERIC_TYPE_NO_OPENGL
    , openGLFormat(openGLFormat_), openGLType(openGLType_)
#endif // NUMERIC_TYPE_NO_OPENGL

#ifndef NUMERIC_TYPE_NO_OPENCV
    , openCVType(openCVType_), openCVTypeNoChannel(openCVTypeNoChannel_)
#endif // NUMERIC_TYPE_NO_OPENCV

#ifndef NUMERIC_TYPE_NO_NIFTI
    , NIFTIType(NIFTIType_)
#endif // NUMERIC_TYPE_NO_NIFTI
{

}

double NumericType::getMin() const
{
    return minMaxLUT[(int)type][0];
}

double NumericType::getMax() const
{
    return minMaxLUT[(int)type][1];
}

#ifndef NUMERIC_TYPE_NO_OPENGL
const NumericType *NumericType::OpenGL(GLenum format, GLenum type)
{
    for (int i = 0; i < numericTypeLUTSize; ++i)
    {
        if (numericTypeLUT[i].openGLFormat == format && numericTypeLUT[i].openGLType == type)
            return &numericTypeLUT[i];
    }

    return NULL;
}
#endif // NUMERIC_TYPE_NO_OPENGL

#ifndef NUMERIC_TYPE_NO_OPENCV
const NumericType *NumericType::OpenCV(int type)
{
    for (int i = 0; i < numericTypeLUTSize; ++i)
    {
        if (numericTypeLUT[i].openCVType == type)
            return &numericTypeLUT[i];
    }

    return NULL;
}

const NumericType *NumericType::OpenCV(int type, int numChannels)
{
    int searchType = CV_MAKETYPE(type, numChannels);
    for (int i = 0; i < numericTypeLUTSize; ++i)
    {
        if (numericTypeLUT[i].openCVType == searchType)
            return &numericTypeLUT[i];
    }

    return NULL;
}
#endif // NUMERIC_TYPE_NO_OPENCV

#ifndef NUMERIC_TYPE_NO_NIFTI
const NumericType *NumericType::NIFTI(int type)
{
    for (int i = 0; i < numericTypeLUTSize; ++i)
    {
        if (numericTypeLUT[i].NIFTIType == type)
            return &numericTypeLUT[i];
    }

    return NULL;
}
#endif // NUMERIC_TYPE_NO_NIFTI
