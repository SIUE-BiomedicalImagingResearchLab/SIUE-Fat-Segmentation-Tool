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

}
