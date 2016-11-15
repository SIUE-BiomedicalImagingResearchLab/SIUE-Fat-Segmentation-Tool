#ifndef NIFTI_HPP
#define NIFTI_HPP

#include <nifti/include/nifti1.h>
#include <nifti/include/fslio.h>

#include <QOpenGLFunctions>
#include <QOpenGLTexture>

static GLenum nifti_to_opengl_LUT[][3] =
{
    {DT_UINT8,      GL_RED,     GL_UNSIGNED_BYTE},
    {DT_INT8,       GL_RED,     GL_BYTE},
    {DT_UINT16,     GL_RED,     GL_UNSIGNED_SHORT},
    {DT_INT16,      GL_RED,     GL_SHORT},
    {DT_UINT32,     GL_RED,     GL_UNSIGNED_INT},
    {DT_INT32,      GL_RED,     GL_INT},
    {DT_FLOAT32,    GL_RED,     GL_FLOAT},
    {DT_RGB24,      GL_RGB,     GL_UNSIGNED_BYTE},
    {DT_RGBA32,     GL_RGBA,    GL_UNSIGNED_BYTE},
    {NULL,          NULL,       NULL}
};

GLenum *nifti_datatype_to_opengl(int datatype);

#endif // NIFTI_HPP
