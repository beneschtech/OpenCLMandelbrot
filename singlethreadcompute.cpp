#include <QDateTime>
#include <iostream>
#include <cmath>
#include "mainwindow.h"
#include "ui_mainwindow.h"

extern double left;
extern double top;
extern double fwidth;
extern double fheight;
extern unsigned int iterations;

void MainWindow::stCompute()
{
  setButtonChecked(dynamic_cast<QPushButton *>(sender()));
  QApplication::setOverrideCursor(Qt::WaitCursor);
  qApp->processEvents();
  int fw = ui->fracImg->frameWidth();
  double start = currentTime();
  QSize sz = ui->fracImg->size() - QSize(fw*2,fw*2);
  uchar *imgData = (uchar *)malloc(sz.width() * sz.height());
  int w = sz.width();
  int h = sz.height();

  for (int y = 0; y < h; y++)
  {
    for (int x = 0; x < w; x++)
    {
      size_t offst = (y*w)+x;
      double xc = left + ((fwidth * x)/w);
      double yc = top + ((fheight * y)/h);
      double xt = 0, yt = 0, xtt,ytt;
      unsigned int i;
      imgData[offst] = 0;
      for (i=0; i < iterations; i++)
      {
        xtt = xt*xt - yt*yt + xc;
        ytt = 2*xt*yt + yc;
        if (xt == xtt && yt == ytt)
          break;
        xt = xtt; yt = ytt;
        if (xt * xt * yt * yt > 16)
        {
          imgData[offst] = i % 256;
          break;
        }
      }
    }
  }

  double ctime = currentTime() - start;

  QImage px(imgData,w,h,w,QImage::Format_Indexed8);
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
