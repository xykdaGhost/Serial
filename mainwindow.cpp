#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QThread>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    myUart(&Uart::getInstance()),
    camera(&GenCamera::getInstance())
{
    ui->setupUi(this);


//    camera->doWork();

    QThread* uartThread = new QThread();
    myUart->moveToThread(uartThread);
    connect(uartThread, SIGNAL(started()), myUart, SLOT(doWork()));
    connect(uartThread, SIGNAL(finished()), uartThread, SLOT(deleteLater()));
    uartThread->start();

//    QThread* cameraThread = new QThread();
//    camera->moveToThread(cameraThread);
//    connect(cameraThread, SIGNAL(started()), camera, SLOT(doWork()));
//    connect(cameraThread, SIGNAL(finished()), cameraThread, SLOT(deleteLater()));
//    cameraThread->start();
//    cameraThread->setPriority(QThread::HighestPriority);

    connect(ui->pushButton, &QPushButton::clicked, this, [=] {
        QString text = "mainwindow thread" + QStringLiteral("%1").arg(quintptr(QThread::currentThread()));
        ui -> label -> setText(text);

    });

    connect(ui->textEdit, &QTextEdit::textChanged, this, [=] {
        QString str = this->ui->textEdit->toPlainText();
        qDebug() << "input " << str;
        i = str.toInt();
        qDebug() << "change : " << i;

    });
    connect(ui->uartButton, &QPushButton::clicked, this, [=] {
        UartRequest::getInstance().sendMessage(i);

    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
