# VisceralFatValidation
This program will take in MRI images in the NIFTI format and display them on the screen. The user will draw a closed contour around specified areas of fat at multiple axial slices. These results are saved in a file to be validated with the VisceralFatSegmentation program. This program is created using Qt and C++.

# Installation
1. Install [Qt Creator](https://www.qt.io/)
  * I recommend using the offline installer and hand selecting which version of Qt Creator to install. There are binaries for particular versions of compilers such as VS 2015 64-bit, VS 2015 32-bit, MinGW, GCC, etc. 
2. [Build External Libraries](https://github.com/addisonElliott/VisceralFatValidation/wiki/Building-External-Libraries-in-Windows)
3. Download VisceralFatValidation
4. Copy custom.pro.sample to custom.pro.  Edit custom.pro to point to the libraries installed in the previous step.
  * A template for custom.pro is given. Anything in brackets should be adjusted(e.g. <OPENCV-PATH>)
5. Open Qt Creator
6. Open VisceralFatValidation.pro project file
7. Run Project

# Dependencies
The following libraries are required to build VisceralFatValidation
* zlib
* NIFTIclib
* OpenCV

### [Building External Libraries in Windows](https://github.com/addisonElliott/VisceralFatValidation/wiki/Building-External-Libraries-in-Windows)
