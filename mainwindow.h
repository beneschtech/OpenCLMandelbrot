#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMap>
#include <QVector>
#include <QPushButton>
#define CL_TARGET_OPENCL_VERSION 220
#define NUM_PLATFORM_MAX 8
#define NUM_DEVICES_MAX 256
#include <CL/cl.h>

namespace Ui {
class MainWindow;
}

struct gpuDef {
  gpuDef();
  ~gpuDef();
  cl_device_id id;
  cl_kernel kernel;
  cl_command_queue cmdQueue;
  cl_context context;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void mousePressEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

private:
    Ui::MainWindow *ui;
    QMap<QString,gpuDef> gpuDefs;
    double currentTime();
    QVector<QPushButton *> pushButtons;
    QPushButton *currentButton;
    void setButtonChecked(QPushButton *);

private slots:
    void stCompute();
    void cpuCompute();
    void gpuCompute();
    void gpuDetect();
};

#endif // MAINWINDOW_H
