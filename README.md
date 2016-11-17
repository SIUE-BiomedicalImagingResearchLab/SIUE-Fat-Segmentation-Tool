# VisceralFatValidation
This program will take in MRI images in the NIFTI format and display them on the screen. The user will draw a closed contour around specified areas of fat at multiple axial slices. These results are saved in a file to be validated with the VisceralFatSegmentation program. This program is created using Qt and C++.

# Prerequisites
In order to be able to compile this program, a few necessary libraries are required. The libraries will have to be built using a compiler of your choice and placed in the appropiate location for Qt Creator to recognize them.

Required Libraries:
	- nifticlib
	- OpenCV
	
# OpenCV
OpenCV versions 3.0 and greater are necessary to build this program. At the time of this writing, I have built it using OpenCV 3.1.0. 

Download the necessary OpenCV version for your computer. Precompiled binaries are okay to use but you can also build OpenCV relatively easily using CMake.
Create a folder for the OpenCV library. In my case, I did C:/Program Files/opencv. Next, create three folders in the OpenCV library just created: include, lib, and bin. include will contain the header files, lib will contain the libraries AND dlls, and bin will contain the DLLs and compiled testing executables. In order to build this project, the DLLs must be in the same folder as the library files.
Copy and paste the appropiate files into your OpenCV library.
View/edit the .pro file in Qt Creator to tell Qt where the library and include folders are located.

# Size Policies in Qt
Laying out a GUI in Qt can be quite daunting if you have never used their format before. However, it is efficient and allows for good-looking GUIs regardless of the platform. Laying out a GUI is done using layouts such as horizontal, vertical, grid, form, etc. By nesting these together and grouping components together, a GUI can be made. Every GUI item in Qt has 2 sizePolicy's attached to it, one for vertical and one for horizontal. They tell the layout manager how the size should be handled. This page is what I found to be the most helpful when trying to understand size policies: http://doc.qt.io/qt-5/qsizepolicy.html