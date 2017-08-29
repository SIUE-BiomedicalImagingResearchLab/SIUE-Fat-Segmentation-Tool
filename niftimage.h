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

class NIFTImage
{
private:
    nifti_image *upper;
    nifti_image *lower;

    SubjectConfig *subConfig;

    int xDim;
    int yDim;
    int zDim;

    cv::Mat data;

public:
    NIFTImage();
    NIFTImage(nifti_image *upper, nifti_image *lower, SubjectConfig *config = NULL);
    NIFTImage(SubjectConfig *config);
    ~NIFTImage();

    bool setImage(nifti_image *upper, nifti_image *lower, SubjectConfig *config = NULL);
    bool setSubjectConfig(SubjectConfig *config);

    bool checkImage() const;
    bool compatible(NIFTImage *image) const;

    nifti_image *getUpperImage() const;
    nifti_image *getLowerImage() const;
    int getXDim() const;
    int getYDim() const;
    int getZDim() const;

    bool isLoaded() const;

    cv::Mat getRegion(std::vector<cv::Range> region, bool clone = false);

    cv::Mat getAxialSlice(int z, bool clone = false);
    cv::Mat getCoronalSlice(int y, bool clone = false);
    cv::Mat getSaggitalSlice(int x, bool clone = false);

    const NumericType *getType() const;

private:
    void orientImage(nifti_image *image, cv::Mat &mat);
};

#endif // NIFTIMAGE_H
