#ifndef TRACING_H
#define TRACING_H

#include <array>
#include <QTime>

#include <opencv2/opencv.hpp>

#include "displayinfo.h"

struct TracingLayerData
{
    cv::Mat data;
    std::vector<QTime> time;
};

struct TracingData
{
    std::array<TracingLayerData, (int)TracingLayer::Count> layers;
};

#endif // TRACING_H
