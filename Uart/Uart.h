#ifndef UART_H
#define UART_H

#include <QObject>
#include <QSerialPort>


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


private:
    explicit Uart();
    QSerialPort * port;
    bool readFlag;
    bool writeFlag;
};

#endif // UART_H
