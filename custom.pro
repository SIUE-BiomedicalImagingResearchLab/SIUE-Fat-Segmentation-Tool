#-------------------------------------------------
#
# Customized file that is ignored by GitHub
# Ideal place for including libraries with absolute paths which will vary between user
#
#-------------------------------------------------

# Include NIFTI files and link dynamic libraries
INCLUDEPATH += 'D:/DevelLibs/nifticlib/buildShared/include'
DEPENDPATH += 'D:/DevelLibs/nifticlib/buildShared/include'

LIBS += -LD:/DevelLibs/nifticlib/buildShared/lib \
        -lnifticdf \
        -lniftiio \
        -lznz

# Include ZLIB files and link dynamic libraries
# (This is used in NIFTI library)
LIBS += -LD:/DevelLibs/zlib-1.2.8/build/lib \
        -lzlib

# Include OpenCV files and link dynamic libraries
# Separate libraries must be used for debug and release mode as this caused a weird issue
# when Release/Debug was mismatched with libraries.
INCLUDEPATH += 'D:/DevelLibs/opencv/build/install/include'
DEPENDPATH += 'D:/DevelLibs/opencv/build/install/include'
CONFIG(debug, debug|release): LIBS += -LD:/DevelLibs/opencv/build/lib/Debug \
        -lopencv_core310d \
        -lopencv_imgproc310d \
        -lopencv_highgui310d \
        -lopencv_ml310d \
        -lopencv_video310d

CONFIG(release, debug|release): LIBS += -LD:/DevelLibs/opencv/build/lib/Release \
        -lopencv_core310 \
        -lopencv_imgproc310 \
        -lopencv_highgui310 \
        -lopencv_ml310 \
        -lopencv_video310
