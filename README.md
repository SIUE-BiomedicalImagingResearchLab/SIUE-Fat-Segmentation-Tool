# VisceralFatValidation
This program will take in MRI images in the NIFTI format and display them on the screen. The user will draw a closed contour around specified areas of fat at multiple axial slices. These results are saved in a file to be validated with the VisceralFatSegmentation program. This program is created using Qt and C++.

# Prerequisites
In order to be able to compile this program, a few necessary libraries are required. The libraries will have to be built using a compiler of your choice and placed in the appropiate location for Qt Creator to recognize them.

Required Libraries:
	- nifticlib
	- OpenCV