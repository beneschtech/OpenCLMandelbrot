#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QProgressDialog>
#include <QMessageBox>

extern double left;
extern double top;
extern double fwidth;
extern double fheight;
extern unsigned int iterations;

void MainWindow::gpuDetect()
{
  QProgressDialog dlg("Detecting OpenCL Devices","Cancel",0,0);
  dlg.show();
  qApp->processEvents();
  cl_platform_id gpuPlatformIds[NUM_PLATFORM_MAX];
  cl_device_id gpuDeviceIds[NUM_DEVICES_MAX];
  cl_uint numPlatforms, numDevices;
  cl_uint gpuDevPtr = 0;

  if ((clGetPlatformIDs(NUM_PLATFORM_MAX,gpuPlatformIds,&numPlatforms) != CL_SUCCESS) || numPlatforms == 0)
  {
    return;
  }
  for (cl_uint platCnt = 0; platCnt < numPlatforms; platCnt++)
  {
    if ((clGetDeviceIDs(gpuPlatformIds[platCnt],CL_DEVICE_TYPE_ALL,NUM_DEVICES_MAX-gpuDevPtr,&gpuDeviceIds[gpuDevPtr],&numDevices) != CL_SUCCESS))
    {
      continue;
    }
    while (numDevices)
    {
      char devName[128];
      memset(devName,0,sizeof(devName));
      clGetDeviceInfo(gpuDeviceIds[gpuDevPtr],CL_DEVICE_NAME,sizeof(devName),devName,NULL);
      QString dName = devName;
      dName.detach();
      if (!gpuDefs.contains(dName))
      {
        gpuDef def;
        def.id = gpuDeviceIds[gpuDevPtr];
        gpuDefs.insert(dName,def);
      }
      numDevices--; gpuDevPtr++;
    }
  }
  dlg.setMaximum(gpuDefs.size());
  QFile kf(":/mandelbrot.cl");
  if (!kf.open(QIODevice::ReadOnly))
  {
    QMessageBox::critical(nullptr,"OpenCL",kf.errorString());
    return;
  }
  QByteArray gpuSrc = kf.readAll();
  const size_t ksz = gpuSrc.size();
  const char *ksrc = gpuSrc.constData();
  kf.close();

  qApp->processEvents();
  auto gpuit = gpuDefs.begin();
  int gpuid = 0;
  cl_int ret;
  QStringList badGPUs;
  for (;gpuit != gpuDefs.end(); gpuit++)
  {
    gpuid++;
    gpuDef *d = &gpuit.value();
    QString devName = gpuit.key();

    d->context = clCreateContext(nullptr,1,&d->id,nullptr,nullptr,&ret);
    if (ret != CL_SUCCESS)
    {
      badGPUs.append(devName);
      continue;
    }
    d->cmdQueue = clCreateCommandQueueWithProperties(d->context,d->id,nullptr,&ret);
    if (ret != CL_SUCCESS)
    {
      badGPUs.append(devName);
      continue;
    }
    cl_program prg = clCreateProgramWithSource(d->context,1,&ksrc,&ksz,&ret);
    if (ret != CL_SUCCESS)
    {
      badGPUs.append(devName);
      continue;
    }
    ret = clBuildProgram(prg,1,&d->id,nullptr,nullptr,nullptr);
    if (ret == CL_BUILD_PROGRAM_FAILURE)
    {
      size_t logSz;
      clGetProgramBuildInfo(prg,d->id,CL_PROGRAM_BUILD_LOG,0,NULL,&logSz);
      char *log = (char *)malloc(logSz+1);
      log[logSz] = 0;
      clGetProgramBuildInfo(prg,d->id,CL_PROGRAM_BUILD_LOG,logSz,log,NULL);
      QMessageBox::warning(NULL,devName,log);
      badGPUs.append(devName);
      continue;
    }
    d->kernel = clCreateKernel(prg,"mandelbrotPoint",&ret);
    if (ret != CL_SUCCESS)
    {
      badGPUs.append(devName);
      continue;
    }

    // If successful, add pushbutton for it
    QPushButton *btn = new QPushButton(ui->centralWidget);
    btn->setObjectName(QString::fromUtf8("gpu%1").arg(gpuid));
    btn->setCheckable(true);
    btn->setText(devName);
    btn->setAutoDefault(true);
    ui->horizontalLayout->addWidget(btn);
    QObject::connect(btn,&QPushButton::clicked,this,&MainWindow::gpuCompute);
    pushButtons.append(btn);
    dlg.setValue(gpuid);
    qApp->processEvents();
  }

  foreach(QString name,badGPUs)
  {
    gpuDefs.remove(name);
  }
}

void MainWindow::gpuCompute()
{
  QPushButton *btn = dynamic_cast<QPushButton *>(sender());
  setButtonChecked(btn);
  gpuDef *d = nullptr;
  if (gpuDefs.contains(btn->text()))
    d = &gpuDefs[btn->text()];
  if (!d)
  {
    QMessageBox::warning(nullptr,"Invalid GPU Configuration","For some reason you have a button, but no configuration... strange");
    return;
  }
  int fw = ui->fracImg->frameWidth();
  QApplication::setOverrideCursor(Qt::WaitCursor);
  qApp->processEvents();
  double start = currentTime();
  QSize sz = ui->fracImg->size() - QSize(fw*2,fw*2);
  int w = sz.width();
  int h = sz.height();
  size_t n_thr = sz.height() * sz.width();
  double params[4];
  params[0] = left;
  params[1] = top;
  params[2] = fwidth;
  params[3] = fheight;
  cl_int ret;
  const size_t imgSz = (sz.width() * sz.height());
  uchar *hImgData = (uchar *)malloc(sz.width() * sz.height());
  cl_mem gParms = clCreateBuffer(d->context,CL_MEM_COPY_HOST_PTR,sizeof(params),params,&ret);
  cl_mem imgData = clCreateBuffer(d->context,CL_MEM_WRITE_ONLY,imgSz,NULL,&ret);
  clSetKernelArg(d->kernel,0,sizeof(cl_mem),(void *)&gParms);
  clSetKernelArg(d->kernel,1,sizeof(int),(void *)&w);
  clSetKernelArg(d->kernel,2,sizeof(int),(void *)&h);
  clSetKernelArg(d->kernel,3,sizeof(int),(void *)&iterations);
  clSetKernelArg(d->kernel,4,sizeof(cl_mem),(void *)&imgData);
  ret = clEnqueueNDRangeKernel(d->cmdQueue,d->kernel,1,NULL,&n_thr,NULL,0,NULL,NULL);
  ret = clEnqueueReadBuffer(d->cmdQueue,imgData,CL_TRUE,0,imgSz,hImgData,0,NULL,NULL);
  clReleaseMemObject(gParms);
  clReleaseMemObject(imgData);

  double ctime = currentTime() - start;

  QImage px(hImgData,w,h,w,QImage::Format_Indexed8);
  px.setColor(0,qRgb(0,0,0));
  for (int i = 1; i < 255; i++)
    px.setColor(i,qRgb(i,i,255-i));

  ui->fracImg->setPixmap(QPixmap::fromImage(px));
  if (ctime > 0.75)
  {
    statusBar()->showMessage(QString("%1 seconds compute time - %2 iterations").arg(ctime).arg(iterations));
  } else if (ctime > 0.01)
  {
    statusBar()->showMessage(QString("%1 ms compute time - %2 iterations").arg(ctime * 1000.0).arg(iterations));
  } else {
    statusBar()->showMessage(QString("%1 us compute time - %2 iterations").arg(ctime * 1000000.0).arg(iterations));
  }
  QApplication::restoreOverrideCursor();
  ui->fracImg->repaint();
}
