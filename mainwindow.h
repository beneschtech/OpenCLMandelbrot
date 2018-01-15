#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QWheelEvent>
#include <CL/cl.h>

namespace Ui {
class MainWindow;
}

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
    cl_kernel gpuKernel;
    cl_command_queue gpuCmdQueue;
    cl_context gpuCtx;

    size_t gpuThreads;
    const char **gpuKernelSrc;

private slots:
    void cpuCompute();
    void gpuCompute();

};

#endif // MAINWINDOW_H
