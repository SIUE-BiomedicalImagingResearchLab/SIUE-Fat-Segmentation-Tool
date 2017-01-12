#ifndef DISPLAYINFO_H
#define DISPLAYINFO_H

enum SliceDisplayType
{
    FatOnly,
    WaterOnly,
    FatFraction,
    WaterFraction,
    FatWater,
    WaterFat
};

enum ColorMap
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
    ":/colormaps/MATLAB_winter.png"
};

enum Location : int
{
    NoChange = -1
};

#endif // DISPLAYINFO_H
