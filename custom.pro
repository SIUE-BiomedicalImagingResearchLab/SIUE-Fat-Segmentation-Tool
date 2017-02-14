#-------------------------------------------------
#
# Customized file that is ignored by GitHub
# Ideal place for including libraries with absolute paths which will vary between user
#
#-------------------------------------------------

# Include NIFTI files and link dynamic libraries
INCLUDEPATH += '<PATH-TO-NIFTICLIB>/include'
DEPENDPATH += '<PATH-TO-NIFTICLIB>/include'

LIBS += -L<PATH-TO-NIFTICLIB>/lib \
        -lnifticdf \
        -lniftiio \
        -lznz

# Include ZLIB files and link dynamic libraries
# (This is used in NIFTI library)
LIBS += -L<PATH-TO-ZLIB>/lib \
        -lzlib

# Include OpenCV files and link dynamic libraries
# Separate libraries must be used for debug and release mode as this caused a weird issue
# when Release/Debug was mismatched with libraries.
INCLUDEPATH += '<PATH-TO-OPENCV>/include'
DEPENDPATH += '<PATH-TO-OPENCV>/include'
CONFIG(debug, debug|release): LIBS += -L<PATH-TO-OPENCV>/lib/Debug \
        -lopencv_core310d \
        -lopencv_imgproc310d \
        -lopencv_highgui310d \
        -lopencv_ml310d \
        -lopencv_video310d

CONFIG(release, debug|release): LIBS += -L<PATH-TO-OPENCV>/lib/Release \
        -lopencv_core310 \
        -lopencv_imgproc310 \
        -lopencv_highgui310 \
        -lopencv_ml310 \
        -lopencv_video310
