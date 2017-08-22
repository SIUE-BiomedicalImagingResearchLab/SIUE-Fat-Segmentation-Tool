# SIUE Fat Segmentation Tool (SFST)
This program consists of a variety of tools within one program to aid in fat segmentation via MRI and ultrasound imaging. As of now, the only tool available in the program is the ability to load in MRI images in the NIFTI format and display the axial and coronal slice views of the MRI. The user is able to trace the various depots of fat such as epicardial, pericardial, peri-aortic, subcutaneous, etc, for each axial slice of the MRI image. The traced user data can then be saved to a TXT file for further processing. This program is created and maintained by Electrical and Computer Engineering (ECE) department at Southern Illinois University Edwardsville (SIUE). This program is created using C++ and the Qt framework.

# [Installing Prebuilt Binaries](https://github.com/addisonElliott/SIUE-Fat-Segmentation-Tool/wiki/Installation)

# Installing from Source
1. Install [Qt Creator](https://www.qt.io/)
    * I recommend using the offline installer and hand selecting which version of Qt Creator to install. There are binaries for particular versions of compilers such as VS 2015 64-bit, VS 2015 32-bit, MinGW, GCC, etc. 
2. [Build External Libraries](https://github.com/addisonElliott/SIUE-Fat-Segmentation-Tool/wiki/Building-External-Libraries)
3. Download SFST source code by cloning the repository or downloading ZIP file
4. To find external libraries, custom.pro is used. This file will be different for each developer depending on where the external libraries are installed and therefore is not tracked by GitHub. Sample custom.pro files are provided for each OS to get you started with building the application. Duplicate the necessary sample custom.pro and rename to custom.pro
    * custom.pro.win.sample - Sample custom.pro file for Windows
        * __Change everything in brackets (e.g. \<OPENCV-PATH\>)__
    * custom.pro.macosx.sample - Sample custom.pro file for Mac OS X
        * __No changes should be required since libraries are assumed to be installed in /usr/local/{bin/lib/include}__
5. Open Qt Creator
6. Open SIUE-Fat-Segmentation-Tool.pro project file
7. Run Project

# [Getting Started with the Software](https://github.com/addisonElliott/SIUE-Fat-Segmentation-Tool/wiki/Getting-Started)
