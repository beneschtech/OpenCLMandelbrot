#include <QThreadPool>
#include <QDateTime>
#include <QImage>
#include <iostream>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cpufractalcomputethread.h"

extern const char *_kernelSrc;
double left = -2.5;
double top = -1.5;
double fwidth = 3.5;
double fheight = 3.0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->cpuPB,SIGNAL(clicked(bool)),this,SLOT(cpuCompute()));
    connect(ui->gpuPB,SIGNAL(clicked(bool)),this,SLOT(gpuCompute()));

    gpuKernelSrc = NULL;
    // Open CL portion
    cl_platform_id gpuPlatformId;
    cl_device_id gpuDeviceId;
    cl_uint numPlatforms, numDevices;
    cl_int ret;

    if ((clGetPlatformIDs(1,&gpuPlatformId,&numPlatforms) != CL_SUCCESS) || numPlatforms == 0)
    {
        QMessageBox::warning(NULL,"OpenCL","No OpenCL capable devices found, or the library is not installed correctly");
        ui->gpuPB->setDisabled(true);
    }
    if ((clGetDeviceIDs(gpuPlatformId,CL_DEVICE_TYPE_GPU,1,&gpuDeviceId,&numDevices) != CL_SUCCESS) || numDevices == 0)
    {
        QMessageBox::warning(NULL,"OpenCL","No OpenCL capable devices found, or the library is not installed correctly");
        ui->gpuPB->setDisabled(true);
    } else {
        char devName[128];
        memset(devName,0,sizeof(devName));
        clGetDeviceInfo(gpuDeviceId,CL_DEVICE_NAME,sizeof(devName),devName,NULL);
        ui->gpuPB->setText(QString("Compute with %1").arg(devName));
    }
    gpuKernelSrc = &_kernelSrc;
    gpuCtx = clCreateContext(NULL,1,&gpuDeviceId,NULL,NULL,&ret);
    gpuCmdQueue = clCreateCommandQueueWithProperties(gpuCtx,gpuDeviceId,NULL,&ret);
    const size_t ksz = strlen(_kernelSrc);
    cl_program prg = clCreateProgramWithSource(gpuCtx,1,gpuKernelSrc,&ksz,&ret);
    ret = clBuildProgram(prg,1,&gpuDeviceId,NULL,NULL,NULL);
    if (ret == CL_BUILD_PROGRAM_FAILURE)
    {
        size_t logSz;
        clGetProgramBuildInfo(prg,gpuDeviceId,CL_PROGRAM_BUILD_LOG,0,NULL,&logSz);
        char *log = (char *)malloc(logSz+1);
        log[logSz] = 0;
        clGetProgramBuildInfo(prg,gpuDeviceId,CL_PROGRAM_BUILD_LOG,logSz,log,NULL);
        QMessageBox::warning(NULL,"OpenCL",log);
        ui->gpuPB->setDisabled(true);
    }
    gpuKernel = clCreateKernel(prg,"mandelbrotPoint",&ret);
}

MainWindow::~MainWindow()
{
    delete ui;
    clFlush(gpuCmdQueue);
    clFinish(gpuCmdQueue);
    clReleaseKernel(gpuKernel);
    clReleaseCommandQueue(gpuCmdQueue);
    clReleaseContext(gpuCtx);
}

void MainWindow::wheelEvent(QWheelEvent *ev)
{
    Qt::MouseButton btn;
    if (ev->angleDelta().y() > 0)
    {
        btn = Qt::LeftButton;
    } else {
        btn = Qt::RightButton;
    }
    QMouseEvent mev(QEvent::MouseButtonPress,ev->pos(),btn,0,0);
    mousePressEvent(&mev);
}

void MainWindow::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
    {
        QPoint p = ui->fracImg->mapFromParent(ev->pos());
        double clx = left + (p.x() * fwidth / double(ui->fracImg->width()));
        double cly = top + (p.y() * fheight / double(ui->fracImg->height()));
        fwidth *= 0.75;
        fheight *= 0.75;
        top = cly - (fheight / 2);
        left = clx - (fwidth / 2);
    } else {
        QPoint p = ui->fracImg->mapFromParent(ev->pos());
        double clx = left + (p.x() * fwidth / double(ui->fracImg->width()));
        double cly = top + (p.y() * fheight / double(ui->fracImg->height()));
        fwidth *= 1.5;
        fheight *= 1.5;
        top = cly - (fheight / 2);
        left = clx - (fwidth / 2);
    }
    if (ui->cpuPB->isChecked())
        cpuCompute();
    if (ui->gpuPB->isChecked())
        gpuCompute();
}

void MainWindow::cpuCompute()
{
    ui->cpuPB->setChecked(true);
    ui->gpuPB->setChecked(false);
    int fw = ui->fracImg->frameWidth();
    qint64 start = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QSize sz = ui->fracImg->size() - QSize(fw*2,fw*2);
    uchar *imgData = (uchar *)malloc(sz.width() * sz.height());
    int w = sz.width();
    int h = sz.height();
    int nThreads = QThreadPool::globalInstance()->maxThreadCount()-1;
    for (int i = 0; i < nThreads; i++)
        QThreadPool::globalInstance()->start(new CPUFractalComputeThread(i,nThreads,w,h,imgData));
    while (QThreadPool::globalInstance()->activeThreadCount() > 1)
        qApp->processEvents();

    double ctime = QDateTime::currentDateTime().toMSecsSinceEpoch() - start;
    ctime /= 1000;
    QImage px(imgData,w,h,w,QImage::Format_Indexed8);
    for (int i = 0; i < 255; i++)
        px.setColor(i,qRgb(i,i,255-i));

    ui->fracImg->setPixmap(QPixmap::fromImage(px));
    statusBar()->showMessage(QString("%1 seconds compute time").arg(ctime));
    ui->fracImg->repaint();
}

void MainWindow::gpuCompute()
{
    ui->cpuPB->setChecked(false);
    ui->gpuPB->setChecked(true);
    int fw = ui->fracImg->frameWidth();
    qint64 start = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QSize sz = ui->fracImg->size() - QSize(fw*2,fw*2);
    int w = sz.width();
    int h = sz.height();
    double params[4];
    params[0] = left;
    params[1] = top;
    params[2] = fwidth;
    params[3] = fheight;
    cl_int ret;
    const size_t imgSz = (sz.width() * sz.height());
    cl_mem gParms = clCreateBuffer(gpuCtx,CL_MEM_COPY_HOST_PTR,sizeof(params),params,&ret);
    cl_mem imgData = clCreateBuffer(gpuCtx,CL_MEM_WRITE_ONLY,imgSz,NULL,&ret);
    clSetKernelArg(gpuKernel,0,sizeof(cl_mem),(void *)&gParms);
    clSetKernelArg(gpuKernel,1,sizeof(int),(void *)&w);
    clSetKernelArg(gpuKernel,2,sizeof(int),(void *)&h);
    clSetKernelArg(gpuKernel,3,sizeof(cl_mem),(void *)&imgData);
    ret = clEnqueueNDRangeKernel(gpuCmdQueue,gpuKernel,1,NULL,&imgSz,NULL,0,NULL,NULL);
    uchar *hImgData = (uchar *)malloc(sz.width() * sz.height());
    ret = clEnqueueReadBuffer(gpuCmdQueue,imgData,CL_TRUE,0,imgSz,hImgData,0,NULL,NULL);
    clReleaseMemObject(gParms);
    clReleaseMemObject(imgData);

    double ctime = QDateTime::currentDateTime().toMSecsSinceEpoch() - start;
    ctime /= 1000;
    QImage px(hImgData,w,h,w,QImage::Format_Indexed8);
    for (int i = 0; i < 255; i++)
        px.setColor(i,qRgb(i,i,255-i));

    ui->fracImg->setPixmap(QPixmap::fromImage(px));
    statusBar()->showMessage(QString("%1 seconds compute time").arg(ctime));
    ui->fracImg->repaint();
}
