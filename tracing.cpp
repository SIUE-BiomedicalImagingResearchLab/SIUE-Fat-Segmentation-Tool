#include "tracing.h"

int TracingLayerData::getXDim() const
{
    return data.size[2];
}

int TracingLayerData::getYDim() const
{
    return data.size[1];
}

int TracingLayerData::getZDim() const
{
    return data.size[0];
}

bool TracingLayerData::isLoaded() const
{
    return data.empty();
}

cv::Mat TracingLayerData::getRegion(std::vector<cv::Range> region, bool clone)
{
    if (data.empty() || region.size() != 3)
        return cv::Mat();

    if (clone)
        return cv::Mat(data, region.data()).clone();
    else
        return cv::Mat(data, region.data());
}

cv::Mat TracingLayerData::getAxialSlice(int z, bool clone)
{
    if (data.empty() || z < 0 || z >= getZDim())
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

cv::Mat TracingLayerData::getCoronalSlice(int y, bool clone)
{
    if (data.empty() || y < 0 || y >= getYDim())
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

cv::Mat TracingLayerData::getSaggitalSlice(int x, bool clone)
{
    if (data.empty() || x < 0 || x >= getXDim())
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

unsigned char &TracingLayerData::at(int x, int y, int z)
{
    return data.at<unsigned char>(z, y, x);
}

void TracingLayerData::set(int x, int y, int z)
{
    data.at<unsigned char>(z, y, x) = 255;
}

void TracingLayerData::reset(int x, int y, int z)
{
    data.at<unsigned char>(z, y, x) = 0;
}

void TracingLayerData::load(int x, int y, int z)
{
    data = cv::Mat({z, y, x}, CV_8UC1, cv::Scalar(0));
    time.resize(z);
}

const NumericType *TracingLayerData::getType() const
{
    if (data.empty())
        return NULL;

    return NumericType::OpenCV(data.type());
}
