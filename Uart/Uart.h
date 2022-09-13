#ifndef UART_H
#define UART_H

#include <QObject>
#include <QSerialPort>
#include <QDateTime>
#include <QStorageInfo>
#include <QProcess>
#include "UartRequest.h"

class Uart : public QObject
{
    Q_OBJECT
public:
    static Uart& getInstance() {
        static Uart uart;
        return uart;
    }
    ~Uart();
    bool getReadFlag();
    bool getWriteFlag();
    void init();


public slots:
    void handle_data();
    void doWork();

    void transmitAquire(int id);


private:
    explicit Uart();
    QSerialPort * port;
    volatile bool readFlag;
    bool writeFlag;

    void on_receive(QByteArray tmpdata);
    void ack_search();
    void ack_heart();
    void ack_control();
    void ack_controlStatus(uchar m);
    void ask_control(uchar m);
    void ack_mode();
    void ack_modeStauts(uchar m);
    void ask_mode(uchar m);
    void ack_date();
    void ask_date();
    void ack_param();
    void ack_paramStatus(int expoTime, int maxExpoTime, int gain, int shootInterval, int svaeInterval, int mode);
    void ask_param(int expoTime, int maxExpoTime, int gain, int shootInterval, int svaeInterval, int mode);
    void ask_result(int kind, int density);
    void ack_save();
    void ack_saveStatus(uchar m);
    void ask_save(uchar m);
    void ack_connect(uchar m);
    void ack_speed();
    void ack_shoot();
    void ack_storage(int size, int num, int hour, int minute, int avliableNum);
    void ack_recovery();
    void ack_grabAgainRequest();

    void closePort();
    void openPort();


};

#endif // UART_H
