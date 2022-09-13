#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Uart/Uart.h"
#include <QThread>
#include "Camera/GenCamera.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Uart * myUart;
    GenCamera * camera;
    int i;
};

#endif // MAINWINDOW_H
