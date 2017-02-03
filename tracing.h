#ifndef TRACING_H
#define TRACING_H

#include <array>
#include <QTime>

#include <opencv2/opencv.hpp>

#include "displayinfo.h"
#include "numerictype.h"

class TracingLayerData
{
public:
    cv::Mat data;
    std::vector<QTime> time;

    int getXDim() const;
    int getYDim() const;
    int getZDim() const;

    bool isLoaded() const;

    cv::Mat getRegion(std::vector<cv::Range> region, bool clone = false);

    cv::Mat getAxialSlice(int z, bool clone = false);
    cv::Mat getCoronalSlice(int y, bool clone = false);
    cv::Mat getSaggitalSlice(int x, bool clone = false);

    unsigned char &at(int x, int y, int z);
    void set(int x, int y, int z);
    void reset(int x, int y, int z);

    const NumericType *getType() const;
};

struct TracingData
{
    std::array<TracingLayerData, (int)TracingLayer::Count> layers;

    TracingLayerData &operator[](std::size_t layer) { return layers[layer]; }
    TracingLayerData &operator[](TracingLayer layer) { return layers[(std::size_t)layer]; }
};

#endif // TRACING_H
