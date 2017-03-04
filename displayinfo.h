#ifndef DISPLAYINFO_H
#define DISPLAYINFO_H

enum class SliceDisplayType : int
{
    FatOnly,
    WaterOnly,
    FatFraction,
    WaterFraction,
    FatWater,
    WaterFat
};

enum class ColorMap : int
{
    Autumn = 0,
    Bone,
    Cool,
    Copper,
    Gray,
    Hot,
    HSV,
    Jet,
    Parula,
    Pink,
    Spring,
    Summer,
    Winter,
    Red,
    Green,
    Blue,
    RedWhite,
    GreenWhite,
    Count
};

static QString colorMapImageName[(int)ColorMap::Count] =
{
    ":/colormaps/MATLAB_autumn.png",
    ":/colormaps/MATLAB_bone.png",
    ":/colormaps/MATLAB_cool.png",
    ":/colormaps/MATLAB_copper.png",
    ":/colormaps/MATLAB_gray.png",
    ":/colormaps/MATLAB_hot.png",
    ":/colormaps/MATLAB_hsv.png",
    ":/colormaps/MATLAB_jet.png",
    ":/colormaps/MATLAB_parula.png",
    ":/colormaps/MATLAB_pink.png",
    ":/colormaps/MATLAB_spring.png",
    ":/colormaps/MATLAB_summer.png",
    ":/colormaps/MATLAB_winter.png",
    ":/colormaps/red.png",
    ":/colormaps/green.png",
    ":/colormaps/blue.png",
    ":/colormaps/redWhite.png",
    ":/colormaps/greenWhite.png"
};

enum Location : int
{
    NoChange = -1
};

enum class TracingLayer : int
{
    EAT = 0,
    IMAT,
    PAAT,
    PAT,
    SCAT,
    VAT,
    Count
};

namespace Dirty
{
    constexpr int Slice                 = 1 << 1,
                  TracesStart           = 1 << 2,
                  TracesEnd             = 1 << (2 + (int)TracingLayer::Count),
                  TracesAll             = (2^((int)TracingLayer::Count) - 1) << 2; // Updates each layer (Typically used when loading trace data)

    constexpr int Trace(TracingLayer layer)
    {
        return TracesStart << (int)layer;
    }
}

enum class WindowViewType : int
{
    AxialCoronalLoRes = 0,
    AxialCoronalHiRes
};

enum class DrawMode : int
{
    Points = 0,
    Erase
};

#endif // DISPLAYINFO_H
