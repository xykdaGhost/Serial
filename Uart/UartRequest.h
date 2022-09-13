#ifndef UARTREQUEST_H
#define UARTREQUEST_H

#include <QObject>

class UartRequest : public QObject
{
    Q_OBJECT
public:
    static UartRequest& getInstance() {
        static UartRequest request;
        return request;
    }
    ~UartRequest();
    void requestNewMessage(int id);

signals:
    void sendMessage(int id);

private:
    explicit UartRequest();
};

#endif // UARTREQUEST_H
