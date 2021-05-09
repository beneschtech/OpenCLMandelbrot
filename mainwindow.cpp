#include <QThreadPool>
#include <QDateTime>
#include <QImage>
#include <iostream>
#include <QMessageBox>
#include <sys/time.h>
#include <QTimer>

#include "mainwindow.h"
#include "ui_mainwindow.h"

extern const char *_kernelSrc;
double left = -2.5;
double top = -1.5;
double fwidth = 3.5;
double fheight = 3.0;
unsigned int iterations = 1024;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  connect(ui->cpuPB,SIGNAL(clicked(bool)),this,SLOT(cpuCompute()));
  connect(ui->stPB,SIGNAL(clicked(bool)),this,SLOT(stCompute()));
  pushButtons.append(ui->cpuPB);
  pushButtons.append(ui->stPB);
  QTimer::singleShot(0,this,&MainWindow::gpuDetect);
  currentButton = nullptr;
}

MainWindow::~MainWindow()
{
  delete ui;
}

gpuDef::gpuDef()
{
  id = nullptr;
  kernel = nullptr;
  cmdQueue = nullptr;
  context = nullptr;
}

gpuDef::~gpuDef()
{
  if (cmdQueue)
  {
    clFlush(cmdQueue);
    clFinish(cmdQueue);
    clReleaseCommandQueue(cmdQueue);
  }
  if (kernel)
    clReleaseKernel(kernel);
  if (context)
    clReleaseContext(context);
  if (id)
    clReleaseDevice(id);
}

void MainWindow::wheelEvent(QWheelEvent *ev)
{
  iterations += ev->angleDelta().y();
  if (currentButton)
    currentButton->click();
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
  if (currentButton)
    currentButton->click();
}

double MainWindow::currentTime()
{
  struct timeval t;
  gettimeofday(&t,NULL);
  double rv = t.tv_sec;
  rv += (t.tv_usec / 1000000.0);
  return rv;
}

void MainWindow::setButtonChecked(QPushButton *pb)
{
  if (!pb)
    return;
  foreach (QPushButton *b,pushButtons)
    b->setChecked(false);
  pb->setChecked(true);
  currentButton = pb;
}
