#include "opencv.h"

#include <QDebug>

namespace opencv
{

void flip(cv::InputArray src, cv::OutputArray dst, int flip_mode)
{
    CV_Assert(flip_mode >= 0 && flip_mode < src.dims());

    if (src.dims() <= 2)
    {
        cv::flip(src, dst, flip_mode);
        return;
    }
    else if (src.empty())
    {
        return;
    }

    cv::Mat srcMat = src.getMat();
    int type = srcMat.type();

    dst.create(srcMat.dims, srcMat.size.p, type);
    cv::Mat dstMat = dst.getMat();

    if (srcMat.size.p[flip_mode] == 1)
    {
        src.copyTo(dst);
        return;
    }

    const uchar *srcFront = srcMat.ptr();
    const uchar *srcBack;

    uchar *dstFront = dstMat.ptr();
    uchar *dstBack;

    const size_t stepAbove = (flip_mode == srcMat.dims - 1) ? 1 : srcMat.step[flip_mode + 1];
    const size_t stepAt = srcMat.step[flip_mode];
    const size_t stepBelow = (flip_mode == 0) ? 1 : srcMat.step[flip_mode - 1];

    //for (int i = 0; i < srcMat.dims; ++i)
    //    qInfo() << "Dim " << i << ": Size=" << srcMat.size[i] << " Step=" << srcMat.step[i];

    const int flipCount = (srcMat.size.p[flip_mode] + 1) / 2;

    int npages = 1;
    for (int i = 0; i < flip_mode; ++i)
        npages *= srcMat.size.p[i];
    const size_t pageInc = flipCount * stepAt;

    for (int i = 0; i < npages; ++i, srcFront += pageInc, srcBack += pageInc,
                                     dstFront += pageInc, dstBack += pageInc)
    {
        srcBack = srcFront + (srcMat.size.p[flip_mode] - 1) * stepAt;
        dstBack = dstFront + (dstMat.size.p[flip_mode] - 1) * stepAt;

        for (int j = 0; j < flipCount; ++j, srcFront += stepAt, srcBack -= stepAt,
                                            dstFront += stepAt, dstBack -= stepAt)
        {
            int k = 0;
            if (((size_t)srcFront | (size_t)dstFront | (size_t)srcBack | (size_t)dstBack) % sizeof(int) == 0)
            {
                for ( ; k <= (int)stepAt - 16; k += 16)
                {
                    int t0 = ((int *)(srcFront + k))[0];
                    int t1 = ((int *)(srcBack + k))[0];

                    ((int *)(dstFront + k))[0] = t1;
                    ((int *)(dstBack + k))[0] = t0;

                    t0 = ((int *)(srcFront + k))[1];
                    t1 = ((int *)(srcBack + k))[1];

                    ((int *)(dstFront + k))[1] = t1;
                    ((int *)(dstBack + k))[1] = t0;

                    t0 = ((int *)(srcFront + k))[2];
                    t1 = ((int *)(srcBack + k))[2];

                    ((int *)(dstFront + k))[2] = t1;
                    ((int *)(dstBack + k))[2] = t0;

                    t0 = ((int *)(srcFront + k))[3];
                    t1 = ((int *)(srcBack + k))[3];

                    ((int *)(dstFront + k))[3] = t1;
                    ((int *)(dstBack + k))[3] = t0;
                }

                for ( ; k <= (int)stepAt - 4; k += 4)
                {
                    int t0 = ((int *)(srcFront + k))[0];
                    int t1 = ((int *)(srcBack + k))[0];

                    ((int *)(dstFront + k))[0] = t1;
                    ((int *)(dstBack + k))[0] = t0;
                }
            }

            for ( ; k < stepAt; k++)
            {
                uchar t0 = srcFront[k];
                uchar t1 = srcBack[k];

                dstFront[k] = t1;
                dstBack[k] = t0;
            }
        }
    }
}


template<typename T> static void
FindNonZeroFunc_(const T* src, size_t *idx_ptr, int len, size_t startIdx)
{
    for(int i = 0; i < len; i++)
    {
        T val = src[i];
        if (val != (T)0)
            *idx_ptr++ = startIdx + i;
    }
}

static void FindNonZeroFunc_8u(const uchar* src, size_t *idx_ptr, int len, size_t startidx)
{ FindNonZeroFunc_(src, idx_ptr, len, startidx ); }

static void FindNonZeroFunc_8s(const schar* src, size_t *idx_ptr, int len, size_t startidx)
{ FindNonZeroFunc_(src, idx_ptr, len, startidx ); }

static void FindNonZeroFunc_16u(const ushort* src, size_t *idx_ptr, int len, size_t startidx)
{ FindNonZeroFunc_(src, idx_ptr, len, startidx ); }

static void FindNonZeroFunc_16s(const short* src, size_t *idx_ptr, int len, size_t startidx)
{ FindNonZeroFunc_(src, idx_ptr, len, startidx ); }

static void FindNonZeroFunc_32s(const int* src, size_t *idx_ptr, int len, size_t startidx)
{ FindNonZeroFunc_(src, idx_ptr, len, startidx ); }

static void FindNonZeroFunc_32f(const float* src, size_t *idx_ptr, int len, size_t startidx)
{ FindNonZeroFunc_(src, idx_ptr, len, startidx ); }

static void FindNonZeroFunc_64f(const double* src, size_t *idx_ptr, int len, size_t startidx)
{ FindNonZeroFunc_(src, idx_ptr, len, startidx ); }

typedef void (*FindNonZeroFunc)(const uchar*, size_t*, int, size_t);

static FindNonZeroFunc getFindNonZeroTab(int depth)
{
    static FindNonZeroFunc findNonZeroTab[] =
    {
        // Use GET_OPTIMIZED macro if I submit code to OpenCV
        (FindNonZeroFunc)(FindNonZeroFunc_8u), (FindNonZeroFunc)(FindNonZeroFunc_8s),
        (FindNonZeroFunc)(FindNonZeroFunc_16u), (FindNonZeroFunc)(FindNonZeroFunc_16s),
        (FindNonZeroFunc)(FindNonZeroFunc_32s),
        (FindNonZeroFunc)(FindNonZeroFunc_32f), (FindNonZeroFunc)(FindNonZeroFunc_64f),
        0
    };

    return findNonZeroTab[depth];
}

// ALREADY DEFINED IN stat.cpp for OpenCV! If I submit this code into OpenCV, remove this code
static void ofs2idx(const cv::Mat& a, size_t ofs, int* idx)
{
    int i, d = a.dims;
    if( ofs > 0 )
    {
        ofs--;
        for( i = d-1; i >= 0; i-- )
        {
            int sz = a.size[i];
            idx[i] = (int)(ofs % sz);
            ofs /= sz;
        }
    }
    else
    {
        for( i = d-1; i >= 0; i-- )
            idx[i] = -1;
    }
}

void findNonZero(cv::InputArray _src, cv::OutputArray _idx)
{
    int type = _src.type(), depth = CV_MAT_DEPTH(type), cn = CV_MAT_CN(type);

    cv::Mat src = _src.getMat();

    int n = cv::countNonZero(src);
    if(n == 0)
    {
        _idx.release();
        return;
    }

    if(_idx.kind() == cv::_InputArray::MAT && !_idx.getMatRef().isContinuous())
        _idx.release();

    _idx.create(n, 1, CV_MAKETYPE(CV_32S, src.dims)); // Number of channels depends on number of dims of cv::Mat
    cv::Mat idx = _idx.getMat();

    CV_Assert(idx.isContinuous());

    FindNonZeroFunc func = getFindNonZeroTab(depth);
    CV_Assert( func != 0 );

    const cv::Mat* arrays[] = {&src, 0};
    uchar* ptrs[1];
    cv::NAryMatIterator it(arrays, ptrs);
    int planeSize = (int)it.size*cn;
    size_t startidx = 1;

    // More efficient way to do this? Is allocating the best?
    size_t *ofs = new size_t[n];

    for( size_t i = 0; i < it.nplanes; i++, ++it, startidx += planeSize )
        func( ptrs[0], ofs, planeSize, startidx );

    int *idx_ptr = reinterpret_cast<int *>(idx.ptr());
    for (int i = 0; i < n; ++i, idx_ptr += src.dims)
        ofs2idx(src, ofs[i], idx_ptr);

    delete ofs;
}

}
