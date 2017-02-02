#ifndef OPENCV_H
#define OPENCV_H

#include <opencv2/opencv.hpp>

namespace opencv
{

void flip(cv::InputArray src, cv::OutputArray dst, int flip_mode);

void findNonZero(cv::InputArray _src, cv::OutputArray _idx);

}

#endif // OPENCV_H
