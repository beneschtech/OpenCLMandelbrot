#include <QDateTime>

#include "cpufractalcomputethread.h"

extern double left;
extern double top;
extern double fwidth;
extern double fheight;
extern unsigned int iterations;

CPUFractalComputeThread::CPUFractalComputeThread(int idx, int tot, int w, int h, uchar *img): QRunnable()
{
    myIdx = idx;
    totalThreads = tot;
    myWidth = w;
    myHeight = h;
    myImgData = img;
}

void CPUFractalComputeThread::run()
{
    double xinc = fwidth / myWidth;
    double yinc = fheight / myHeight;

    for (int x=myIdx; x < myWidth; x+=totalThreads)
    {
        for (int y = 0; y < myHeight; y++)
        {
            double xc = left + (xinc * x);
            double yc = top + (yinc * y);
            double xt = 0,yt = 0,xtt,ytt;
            unsigned int i;
            for (i = 0; i < iterations; i++)
            {
                xtt = xt*xt - yt*yt + xc;
                ytt = 2*xt*yt + yc;
                xt = xtt; yt = ytt;
                if (xt * xt * yt * yt > 16)
                    break;
            }
            i = i % 256;
            myImgData[(y*(myWidth)) + x] = i;
        }
    }
}
