#ifndef UART_H
#define UART_H
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>
#include "usertype.h"

class Uart: public QObject
{
    Q_OBJECT
public:
    explicit Uart(QObject* parent = 0);
    ~Uart();
    static quint16 CRCCheck(QByteArray data);
private:
    QList<QSerialPort*> uartList;
    QList<QSerialPortInfo> spiList;
    QTimer* flushTimer;

public slots:
    int openXUart(QString name, uart_param_t);
    int closeXUart(QString name);
    void onSendData(QString name, QByteArray);
    void onFlushTimerTimeout();
    void readUartData();
signals:
    newUartPlugin(QStringList name, QStringList factory);
    newDataComming(QString name, QByteArray data);
    logMessage(QString);
};

#endif // UART_H
