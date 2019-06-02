#include "uart.h"
#include <QDebug>

Uart::Uart(QObject* parent)
    :QObject(parent)
{
    flushTimer = new QTimer(this);
    connect(flushTimer, &QTimer::timeout, this, &Uart::onFlushTimerTimeout);
    flushTimer->start(1000);
    uartList.clear();
}
Uart::~Uart()
{
    foreach (QSerialPort* s, uartList) {
        QString uartName = s->portName();
        s->close();
        delete s;
        qDebug() << "Delete: " << uartName;
    }
}

void Uart::onFlushTimerTimeout()
{
    QList<QSerialPortInfo> tempInfo = QSerialPortInfo::availablePorts();
    QStringList oldList;
    QStringList nowList;
    foreach(QSerialPortInfo info, tempInfo) {
        nowList.append(info.portName());
    }
    foreach(QSerialPortInfo info, spiList) {
        oldList.append(info.portName());
    }
    if (oldList != nowList) {
        QStringList name, manufacturer;
        foreach(QSerialPortInfo info, tempInfo) {
            name.append(info.portName());
            manufacturer.append(info.manufacturer());
        }
        emit newUartPlugin(name, manufacturer);
        spiList = tempInfo;
    }
}
int Uart::openXUart(QString name, uart_param_t p)
{
    QSerialPort *s = new QSerialPort(name.split("-").at(0));
    s->setBaudRate(p.baudRate);
    s->setDataBits(p.dataBits);
    s->setStopBits(p.stopBits);
    s->setParity(p.parity);
    if (s->open(QSerialPort::ReadWrite)) {
        s->flush();
        connect(s, &QSerialPort::readyRead, this, &Uart::readUartData);
        emit logMessage("Open uart success");
        uartList.append(s);
        return 0;
    } else {
        delete s;
        s = 0;
        emit logMessage("Open uart failure");
        return -1;
    }
}
int Uart::closeXUart(QString name)
{
    foreach(QSerialPort* sp, uartList) {
        if (name.contains(sp->portName(), Qt::CaseInsensitive)) {
            sp->close();
            uartList.removeOne(sp);
            emit logMessage("Close uart success");
            return 0;
        }
    }
    return 0;
}
void Uart::onSendData(QString name, QByteArray d)
{
    foreach (QSerialPort* sp, uartList) {
        if (name.contains(sp->portName(), Qt::CaseInsensitive)) {
            sp->write(d);
            sp->waitForBytesWritten(20);
            emit logMessage(QString("%1: %2").arg(sp->portName()).arg(QString(d.toHex())));
        } else {
            emit logMessage("Please open uart first!");
        }
    }
}
void Uart::readUartData()
{
    foreach(QSerialPort *sp, uartList) {
        int bytesToRead = 0;
        if ((bytesToRead = sp->bytesAvailable()) > 0) {
            emit newDataComming(sp->portName(), sp->readAll());
        }
    }
}

quint16 Uart::CRCCheck(QByteArray data)
{
    quint16 i, j, tmp, crc16 = 0xffff;
    for (i = 0; i < data.size(); i++) {
        crc16 = quint8(data.at(i))^crc16;
        for (j = 0; j < 8; j++) {
            tmp=crc16 & 0x0001;
            crc16 = crc16 >> 1;
            if (tmp) crc16 = crc16 ^ 0xa001;
        }
    }
    return crc16;
}
