#include "nifti.hpp"

GLenum *nifti_datatype_to_opengl(int datatype)
{
    for (int i = 0; nifti_to_opengl_LUT[i][0] != NULL; i++)
    {
        if (nifti_to_opengl_LUT[i][0] == datatype)
            return nifti_to_opengl_LUT[i];
    }

    return NULL;
}
