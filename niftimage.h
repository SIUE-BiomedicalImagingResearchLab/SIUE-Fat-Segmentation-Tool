#ifndef NIFTIMAGE_H
#define NIFTIMAGE_H

#include <QDebug>
#include <QFile>

#include <opencv2/opencv.hpp>
#include "opencv.h"
#include "util.h"
#include "subjectconfig.h"
#include "numerictype.h"

#include <QOpenGLFunctions_3_3_Core>

#include <nifti/nifti1.h>
#include <nifti/nifti1_io.h>

static GLenum niftiToOpenGLLUT[][3] =
{
    {DT_UINT8,      GL_RED,     GL_UNSIGNED_BYTE},
    {DT_INT8,       GL_RED,     GL_BYTE},
    {DT_UINT16,     GL_RED,     GL_UNSIGNED_SHORT},
    {DT_INT16,      GL_RED,     GL_SHORT},
    {DT_UINT32,     GL_RED,     GL_UNSIGNED_INT},
    {DT_INT32,      GL_RED,     GL_INT},
    {DT_FLOAT32,    GL_RED,     GL_FLOAT},
    {DT_RGB24,      GL_RGB,     GL_UNSIGNED_BYTE},
    {DT_RGBA32,     GL_RGBA,    GL_UNSIGNED_BYTE},
    NULL
};

static int niftiToOpenCVLUT[][2] =
{
    {DT_UINT8,      CV_8UC1},
    {DT_INT8,       CV_8SC1},
    {DT_UINT16,     CV_16UC1},
    {DT_INT16,      CV_16SC1},
    {DT_INT32,      CV_32SC1},
    {DT_FLOAT32,    CV_32FC1},
    {DT_RGB24,      CV_8UC3},
    {DT_RGBA32,     CV_8UC4},
    NULL
};

static GLenum OpenCVToOpenGLLUT[][3] =
{
    {CV_8UC1,       GL_RED,     GL_UNSIGNED_BYTE},
    {CV_8SC1,       GL_RED,     GL_BYTE},
    {CV_16UC1,      GL_RED,     GL_UNSIGNED_SHORT},
    {CV_16SC1,      GL_RED,     GL_SHORT},
    {CV_32SC1,      GL_RED,     GL_INT},
    {CV_32FC1,      GL_RED,     GL_FLOAT},
    {CV_8UC3,       GL_RGB,     GL_UNSIGNED_BYTE},
    {CV_8UC4,       GL_RGBA,    GL_UNSIGNED_BYTE},
    NULL
};

class NIFTImage
{
private:
    nifti_image *upper;
    nifti_image *lower;

    int xDim;
    int yDim;
    int zDim;

    cv::Mat data;

    SubjectConfig *subConfig;

public:
    NIFTImage();
    NIFTImage(nifti_image *upper, nifti_image *lower, SubjectConfig *config = NULL);
    NIFTImage(SubjectConfig *config);
    ~NIFTImage();

    bool setImage(nifti_image *upper, nifti_image *lower, SubjectConfig *config = NULL);
    bool setSubjectConfig(SubjectConfig *config);

    bool checkImage();
    bool compatible(NIFTImage *image);

    nifti_image *getUpperImage();
    nifti_image *getLowerImage();
    int getXDim();
    int getYDim();
    int getZDim();

    cv::Mat getRegion(std::vector<cv::Range> region, bool clone = false);

    cv::Mat getAxialSlice(int z, bool clone = false);
    cv::Mat getCoronalSlice(int y, bool clone = false);
    cv::Mat getSaggitalSlice(int x, bool clone = false);

    NumericType *getType();
};

#endif // NIFTIMAGE_H
