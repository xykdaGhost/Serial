#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    myUart(&Uart::getInstance())
{
    ui->setupUi(this);

    QThread* uartThread = new QThread();
    myUart->init();
    myUart->moveToThread(uartThread);
    connect(uartThread, SIGNAL(started()), myUart, SLOT(doWork()));
    connect(uartThread, SIGNAL(finished()), uartThread, SLOT(deleteLater()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
