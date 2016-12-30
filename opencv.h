#ifndef OPENCV_H
#define OPENCV_H

#include <opencv2/opencv.hpp>

namespace opencv
{

void flip(cv::InputArray src, cv::OutputArray dst, int flip_mode);

}

#endif // OPENCV_H
