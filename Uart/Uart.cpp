#include "Uart.h"
#include <QDebug>
#include <QThread>


Uart::Uart()
{

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
    QByteArray data = port->readAll();
//    qDebug() << "receive : " << data.toHex();
    on_receive(data);
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
    port = new QSerialPort();
    init();
    connect(&UartRequest::getInstance(), SIGNAL(sendMessage(int)), this, SLOT(transmitAquire(int)), Qt::QueuedConnection);
//    while (true)
//    {

//        if (readFlag)
//        {
//            QByteArray data = port->readAll();
//            qDebug() << "getData: " << data.toHex();
//            readFlag = false;

//            on_receive(data);
//            ack_search();
//        }
//    }
}

char BCDtoUINT (char p) {
    return ((p>>4)*10 + (p&0x0f));
}

void Uart::on_receive(QByteArray tmpdata) {

    if (tmpdata[0] == 0xea) {
        if (tmpdata[1] == 0x02) {
            if (tmpdata[2] == 0x00) {
                if (tmpdata[3] == 0x90) {
                    ack_shoot();
                }
                else if (tmpdata[3] == 0x11) {
                    ack_controlStatus(1);
                }
                else if (tmpdata[3] == 0x23) {
                    ack_modeStauts(1);
                }
                else if (tmpdata[3] == 0x72) {
                    ack_saveStatus(2);
                }
                else if (tmpdata[3] == 0x53) {
                    ack_paramStatus(2000,
                                                    1000,
                                                    50,
                                                    4000,
                                                    4000,
                                                    1);
                }
                else if (tmpdata[3] == 0x80) {
                    ack_speed();

                }
                else if (tmpdata[3] == 0xb1) {
//                    QStorageInfo storage= QStorageInfo::mountedVolumes().at(3);
//                    storage.refresh();
//                    if (storage.isReadOnly()) {
//                        qDebug() << "isReadOnly:" << storage.isReadOnly();
//                    }
//                    QString text = tr("%1").arg(storage.bytesAvailable()/1000/1000/1000) + "GB";

//                    GLOBAL_STORAGE = storage.bytesAvailable()/1000/1000/1000;

//                    QDateTime now = QDateTime::currentDateTime();

//                    int seconds = GLOBAL_STARTTIME.secsTo(now);


//                    int hours = seconds / 3600;
//                    seconds -= hours * 3600;
//                    int mins = seconds /60;


//                    int left = GLOBAL_STORAGE * 1024 /  18;
                    ack_storage(100, 100, 1, 1, 1);
                }
                else if (tmpdata[3] == 0xb5) {
                    ack_recovery();
                }
            }
            else if (tmpdata[2] == 0x63) {
                if (tmpdata[3] == 0x01) {
                    ack_search();
                }
                else if (tmpdata[3] == 0x20) {
                    ack_heart();
                }
            }
            else if (tmpdata[2] == 0x07) {
                    ack_connect(1);
            }


        }
        else if (tmpdata[1] == 0x0a) {

            if (tmpdata[3] == 0x41) {

                QString date = "sudo date -s \"" +
                QString::number(BCDtoUINT(tmpdata.data()[4])*100 + BCDtoUINT(tmpdata.data()[5])) + "-" +
                QString::number(BCDtoUINT(tmpdata.data()[6])) + "-" +
                QString::number(BCDtoUINT(tmpdata.data()[7])) + " " +
                QString::number(BCDtoUINT(tmpdata.data()[9])) + ":" +
                QString::number(BCDtoUINT(tmpdata.data()[10])) + ":" +
                QString::number(BCDtoUINT(tmpdata.data()[11])) + "\"";
                qDebug() << date;

                QProcess::startDetached(date);
                QProcess::startDetached("hwclock -w");
                QProcess::startDetached("sync");
                static bool flag = true;
                if (flag){
                   flag = false;
                //                GLOBAL_STARTTIME = QDateTime::currentDateTime();
                }
                ack_date();
            }



        }
        else if (tmpdata[1] == 0x03) {
            if (tmpdata[3] == 0x20) {

                ack_mode();
            }
            else if (tmpdata[3] == 0x10) {
                ack_control();



            }
            else if (tmpdata[3] == 0x11) {

                    ack_controlStatus(2);


            }
            else if (tmpdata[3] == 0x80) {

                ack_speed();
            }
            else if (tmpdata[3] == 0x70) {

                ack_save();
            }

        }
        else if (tmpdata[1] == 0x0c) {

                ack_param();

            }


    }
}


void Uart::ack_search() {
    QByteArray data;
    data.resize(6);
    data[0] = 0xea;
    data[1] = 0x02;
    data[2] = 0xe3;
    data[3] = 0x01;
    data[4] = (data[2]+data[3])%256;
    data[5] = 0xeb;
    qDebug() << "write thread is :" << QThread::currentThread();
    qDebug() << "can write ? :" << port->isWritable();
    port->write(data, 6);

    //qDebug() << "ack serach";
}

void Uart::ack_heart() {
    QByteArray data;
    data.resize(6);
    data[0] = 0xea;
    data[1] = 0x02;
    data[2] = 0xe3;
    data[3] = 0x20;
    data[4] = (data[2]+data[3])%256;
    data[5] = 0xeb;
    port->write(data, 6);

//    qDebug() << "heart checked";
}

void Uart::ack_control() {
    QByteArray data;
    data.resize(6);
    data[0] = 0xea;
    data[1] = 0x02;
    data[2] = 0x80;
    data[3] = 0x10;
    data[4] = (data[2]+data[3])%256;
    data[5] = 0xeb;
    port->write(data, 6);

//    qDebug() << "ack control";
}

void Uart::ack_controlStatus(uchar m) {
    QByteArray data;
    data.resize(7);
    data[0] = 0xea;
    data[1] = 0x03;
    data[2] = 0x80;
    data[3] = 0x11;
    data[4] = m;
    data[5] = (data[2]+data[3]+data[4])%256;
    data[6] = 0xeb;
    port->write(data, 7);

//    qDebug() << "ack control status";
}

void Uart::ask_control(uchar m) {
    QByteArray data;
    data.resize(7);
    data[0] = 0xea;
    data[1] = 0x03;
    data[2] = 0x00;
    data[3] = 0x11;
    data[4] = m;
    data[5] = (data[2]+data[3]+data[4])%256;
    data[6] = 0xeb;
    port->write(data, 7);

//    qDebug() << "ask control";
}

void Uart::ack_mode() {
    QByteArray data;
    data.resize(6);
    data[0] = 0xea;
    data[1] = 0x02;
    data[2] = 0x80;
    data[3] = 0x20;
    data[4] = 0xa0;
    data[5] = 0xeb;
    port->write(data, 6);

//    qDebug() << "ack mode";
}

void Uart::ack_modeStauts(uchar m) {
    QByteArray data;
    data.resize(7);
    data[0] = 0xea;
    data[1] = 0x03;
    data[2] = 0x80;
    data[3] = 0x23;
    data[4] = m;
    data[5] = (data[2]+data[3]+data[4])%256;
    data[6] = 0xeb;
    port->write(data, 7);

//    qDebug() << "ack mode status";
}

void Uart::ask_mode(uchar m) {
    QByteArray data;
    data.resize(7);
    data[0] = 0xea;
    data[1] = 0x03;
    data[2] = 0x00;
    data[3] = 0x26;
    data[4] = m;
    data[5] = (data[2]+data[3]+data[4])%256;
    data[6] = 0xeb;
    port->write(data, 7);

//    qDebug() << "ask mode";
}

void Uart::ack_date() {
    QByteArray data;
    data.resize(6);
    data[0] = 0xea;
    data[1] = 0x02;
    data[2] = 0x80;
    data[3] = 0x40;
    data[4] = 0xc0;
    data[5] = 0xeb;
    port->write(data, 6);

//    qDebug() << "ack date";
}

void Uart::ask_date() {
    QByteArray data;
    data.resize(6);
    data[0] = 0xea;
    data[1] = 0x02;
    data[2] = 0x00;
    data[3] = 0x41;
    data[4] = 0x41;
    data[5] = 0xeb;
    port->write(data, 6);

//    qDebug() << "ask date";
}

void Uart::ack_param() {
    QByteArray data;
    data.resize(6);
    data[0] = 0xea;
    data[1] = 0x02;
    data[2] = 0x80;
    data[3] = 0x50;
    data[4] = 0xd0;
    data[5] = 0xeb;
    port->write(data, 6);

//    qDebug() << "ack param";
}

void Uart::ack_paramStatus(int expoTime, int maxExpoTime, int gain, int shootInterval, int svaeInterval, int mode) {
    QByteArray data;
    data.resize(16);
    data[0] = 0xea;
    data[1] = 0x12;
    data[2] = 0x80;
    data[3] = 0x53;
    data[4] = (uchar)(expoTime / 256);
    data[5] = (uchar)(expoTime % 256);
    data[6] = (uchar)(maxExpoTime / 256);
    data[7] = (uchar)(maxExpoTime % 256);
    data[8] = (uchar)gain;
    data[9] = (uchar)(shootInterval / 256);
    data[10] = (uchar)(shootInterval % 256);
    data[11] = (uchar)(svaeInterval / 256);
    data[12] = (uchar)(svaeInterval % 256);
    data[13] = (uchar)mode;
    data[14] = (data[2]+data[3]+data[4]+data[5]+data[6]+data[7]+data[8]+data[9]+data[10]+data[11]+data[12]+data[13])%256;
    data[15] = 0xeb;
    port->write(data, 16);

//    qDebug() << "ack param status";
}

void Uart::ask_param(int expoTime, int maxExpoTime, int gain, int shootInterval, int svaeInterval, int mode) {
    QByteArray data;
    data.resize(16);
    data[0] = 0xea;
    data[1] = 0x12;
    data[2] = 0x00;
    data[3] = 0x58;
    data[4] = (uchar)(expoTime / 256);
    data[5] = (uchar)(expoTime % 256);
    data[6] = (uchar)(maxExpoTime / 256);
    data[7] = (uchar)(maxExpoTime % 256);
    data[8] = (uchar)gain;
    data[9] = (uchar)(shootInterval / 256);
    data[10] = (uchar)(shootInterval % 256);
    data[11] = (uchar)(svaeInterval / 256);
    data[12] = (uchar)(svaeInterval % 256);
    data[13] = (uchar)mode;
    data[14] = (data[2]+data[3]+data[4]+data[5]+data[6]+data[7]+data[8]+data[9]+data[10]+data[11]+data[12]+data[13])%256;
    data[15] = 0xeb;
    port->write(data, 16);

//    qDebug() << "ask param";
}

void Uart::ask_result(int kind, int density) {
    QByteArray data;
    data.resize(6);
    data[0] = 0xea;
    data[1] = 0x04;
    data[2] = 0x00;
    data[3] = 0x62;
    data[4] = (uchar)kind;
    data[5] = (uchar)density;
    data[6] = (data[2] + data[3] + data[4] +data[5])%256;
    data[7] = 0xeb;
    port->write(data, 8);

//    qDebug() << "ask result" << "kind: " << kind << "density: " << density ;
}

void Uart::ack_save() {
    QByteArray data;
    data.resize(6);
    data[0] = 0xea;
    data[1] = 0x02;
    data[2] = 0x80;
    data[3] = 0x70;
    data[4] = 0xf0;
    data[5] = 0xeb;
    port->write(data, 6);

//    qDebug() << "ack save";
}

void Uart::ack_saveStatus(uchar m) {
    QByteArray data;
    data.resize(7);
    data[0] = 0xea;
    data[1] = 0x03;
    data[2] = 0x80;
    data[3] = 0x72;
    data[4] = m;
    data[5] = (data[2] + data[3] + data[4])%256;
    data[6] = 0xeb;
    port->write(data, 7);

//    qDebug() << "ack save status";;
}

void Uart::ask_save(uchar m) {
    QByteArray data;
    data.resize(7);
    data[0] = 0xea;
    data[1] = 0x03;
    data[2] = 0x00;
    data[3] = 0x75;
    data[4] = m;
    data[5] = (data[2] + data[3] + data[4])%256;
    data[6] = 0xeb;
    port->write(data, 7);

//    qDebug() << "ack save status";;
}

void Uart::ack_connect(uchar m) {
    QByteArray data;
    data.resize(7);
    data[0] = 0xea;
    data[1] = 0x03;
    data[2] = 0x87;
    data[3] = 0x02;
    data[4] = m;
    data[5] = (data[2] + data[3] + data[4])%256;
    data[6] = 0xeb;
    port->write(data, 7);

    qDebug() << "ack save status";
}

void Uart::ack_speed() {
    QByteArray data;
    data.resize(6);
    data[0] = 0xea;
    data[1] = 0x02;
    data[2] = 0x80;
    data[3] = 0x80;
    data[4] = 0x00;
    data[5] = 0xeb;
    port->write(data, 6);

//   qDebug() << "ack speed";
}

void Uart::ack_shoot() {
    QByteArray data;
    data.resize(6);
    data[0] = 0xea;
    data[1] = 0x02;
    data[2] = 0x80;
    data[3] = 0x90;
    data[4] = (data[2] + data[3])%256;
    data[5] = 0xeb;
    port->write(data, 6);

    qDebug() << "ack_shoot success";
}

void Uart::ack_storage(int size, int num, int hour, int minute, int avliableNum) {
    QByteArray data;
    data.resize(14);
    data[0] = 0xea;
    data[1] = 0x0a;
    data[2] = 0x80;
    data[3] = 0xb1;
    data[4] = (uchar)(size/256);
    data[5] = (uchar)(size%256);
    data[6] = (uchar)(num/256);
    data[7] = (uchar)(num%256);
    data[8] = (uchar)hour;
    data[9] = (uchar)minute;
    data[10] = (uchar)(avliableNum/256);
    data[11] = (uchar)(avliableNum%256);
    data[12] = (data[2]+data[3]+data[4]+data[5]+data[6]+data[7]+data[8]+data[9]+data[10]+data[11])%256;
    data[13] = 0xeb;
    port->write(data, 14);

//    qDebug() << "ack storage";
}

void Uart::ack_recovery() {
    QByteArray data;
    data.resize(6);
    data[0] = 0xea;
    data[1] = 0x02;
    data[2] = 0x80;
    data[3] = 0xb5;
    data[4] = (data[2]+data[3])%256;
    data[5] = 0xeb;
    port->write(data, 6);

//    qDebug() << "ack recovery";
}

void Uart::ack_grabAgainRequest() {
    QByteArray data;
    data.resize(6);
    data[0] = 0xea;
    data[1] = 0x03;
    data[2] = 0x00;
    data[3] = 0xb8;
    data[4] = (data[2]+data[3])%256;
    data[5] = 0xeb;
    port->write(data, 6);

    qDebug() << "request again";
}

void Uart::transmitAquire(int id) {
    switch (id) {
    case 1:
//        ack_search();
//        ack_heart();
//        ack_control();
//        ack_controlStatus(uchar m);
        ask_control(2);
//        ack_mode();
//        ack_modeStauts(uchar m);
//        ask_mode(uchar m);
//        ack_date();
//        ask_date();
//        ack_param();
//        ack_paramStatus(int expoTime, int maxExpoTime, int gain, int shootInterval, int svaeInterval, int mode);
//        ask_param(int expoTime, int maxExpoTime, int gain, int shootInterval, int svaeInterval, int mode);
//        ask_result(int kind, int density);
//        ack_save();
//        ack_saveStatus(uchar m);
//        ask_save(uchar m);
//        ack_connect(uchar m);
//        ack_speed();
//        ack_shoot();
//        ack_storage(int size, int num, int hour, int minute, int avliableNum);
//        ack_recovery();
//        ack_grabAgainRequest();

        break;
    default:
        break;
    }
}
