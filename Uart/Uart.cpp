#include "Uart.h"
#include <QDebug>
#include <QThread>

Uart::Uart()
{
    port = new QSerialPort();
    readFlag = false;
    writeFlag = false;
}

Uart::~Uart()
{
    port->close();
    port->deleteLater();
}

void Uart::init()
{
    port->setPortName("/dev/ttyTHS0");
    port->setBaudRate(QSerialPort::Baud115200);
    port->setDataBits(QSerialPort::Data8);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);
    port->setParity(QSerialPort::OddParity);

    if (port->open(QIODevice::ReadWrite)) {
        qDebug() << "Port have been opened";
    } else {
        qDebug() << "open it failed";
    }
    connect(port, SIGNAL(readyRead()), this, SLOT(handle_data()), Qt::DirectConnection);
}

void Uart::handle_data()
{
    this->readFlag = true;
}

bool Uart::getReadFlag()
{
    return readFlag;
}

bool Uart::getWriteFlag()
{
    return writeFlag;
}

void Uart::doWork()
{
    while (true)
    {
        if (readFlag)
        {
            QByteArray data = port->readAll();
            qDebug() << "readThread is : " << QThread::currentThreadId();
        }
    }
}
