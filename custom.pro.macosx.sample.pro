#-------------------------------------------------
#
# Customized file that is ignored by GitHub
# Ideal place for including libraries with absolute paths which will vary between user
#
#-------------------------------------------------

INCLUDEPATH += /usr/local/include

LIBS += -L/usr/local/lib \
        -lnifticdf.2.0.0 \
        -lniftiio.2.0.0 \
        -lznz.2.0.0

# Include ZLIB files and link dynamic libraries
# (This is used in NIFTI library)
LIBS += -L/usr/local/lib -lz

# Include OpenCV files and link dynamic libraries
# Separate libraries must be used for debug and release mode as this caused a weird issue
# when Release/Debug was mismatched with libraries.
LIBS += -L/usr/local/lib \
        -lopencv_core.3.2.0 \
        -lopencv_imgproc.3.2.0 \
        -lopencv_highgui.3.2.0 \
        -lopencv_video.3.2.0

# Flags that disable particular warnings in Clang compilers
# I found the warnings not helpful and tedious to manage when building
QMAKE_CXXFLAGS_WARN_ON += -Wno-reorder
QMAKE_CXXFLAGS_WARN_ON += -Wno-missing-braces
QMAKE_CXXFLAGS_WARN_ON += -Wno-switch
