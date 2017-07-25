# VisceralFatValidation
This program will take in MRI images in the NIFTI format and display them on the screen. The user will draw a closed contour around specified areas of fat at multiple axial slices. These results are saved in a file to be validated with the VisceralFatSegmentation program. This program is created using Qt and C++.

# Installation
1. Install [Qt Creator](https://www.qt.io/)
    * I recommend using the offline installer and hand selecting which version of Qt Creator to install. There are binaries for particular versions of compilers such as VS 2015 64-bit, VS 2015 32-bit, MinGW, GCC, etc. 
2. [Build External Libraries](https://github.com/addisonElliott/VisceralFatValidation/wiki/Building-External-Libraries)
3. Download VisceralFatValidation library
4. To find external libraries, VisceralFatValidation uses custom.pro. This file will be different for each user depending on where the external libraries are installed and therefore is not tracked by GitHub. Sample custom.pro files are provided for each OS to get you started with building the application. Duplicate the necessary sample custom.pro and rename to custom.pro
    * custom.pro.win.sample - Sample custom.pro file for Windows
        * __Change everything in brackets (e.g. \<OPENCV-PATH\>)__
    * custom.pro.macosx.sample - Sample custom.pro file for Mac OS X
        * __No changes should be required since libraries are assumed to be installed in /usr/local/{bin/lib/include}__
5. Open Qt Creator
6. Open VisceralFatValidation.pro project file
7. Run Project

# Dependencies
The following libraries are required to build VisceralFatValidation
* zlib
* NIFTIclib
* OpenCV

### [Building External Libraries](https://github.com/addisonElliott/VisceralFatValidation/wiki/Building-External-Libraries)
