#ifndef CPUFRACTALCOMPUTETHREAD_H
#define CPUFRACTALCOMPUTETHREAD_H

#include <QLabel>
#include <QPixmap>
#include <QRunnable>
#include <QPainter>
#include <QStatusBar>

class CPUFractalComputeThread : public QRunnable
{
public:
    CPUFractalComputeThread(int idx,int tot,int w, int h,uchar *img);
    void run();

private:
    int myIdx;
    int totalThreads;
    int myWidth;
    int myHeight;
    uchar *myImgData;

};

#endif // CPUFRACTALCOMPUTETHREAD_H
