#ifndef DISPLAYINFO_H
#define DISPLAYINFO_H

namespace Dirty
{
    constexpr int Slice = 0x01,
                  Crosshair = 0x02,
                  Traces = 0x04, // Updates only the active layer of traces
                  TracesAll = 0x08; // Updates each layer (Typically used when loading trace data)
}

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

static QString colorMapImageName[ColorMap::Count] =
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
