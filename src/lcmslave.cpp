#include "lcmslave.h"
#include <QDebug>
#include "usertype.h"
#include "uart.h"

LcmSlave::~LcmSlave()
{

}
void LcmSlave::getDataFromUart(QString, QByteArray)
{

}
void LcmSlave::onCycleSendUartDataTimerTimeout()
{
    if (uartName.isEmpty()) return;
    QByteArray temp;
    emit collectChargerData(0, temp);
    for (int i = 0; i < 2; i++) {
        QByteArray d = temp.mid(i*sizeof(lcm_slave_mcu_charging_data_t), sizeof(lcm_slave_mcu_charging_data_t));
        QByteArray head;
        head.append(char(0x68)).append(char(i+1)).append(char(0x67)).append(char(sizeof(lcm_slave_mcu_charging_data_t)));
        d.prepend(head);
        quint16 crc = Uart::CRCCheck(d);
        d.append(char(crc&0xff)).append(char(crc>>8));
        emit sendDataToUart(uartName, d);
    }
}
void LcmSlave::onGetUartName(QString who, QString name)
{
    if (who == this->objectName()) {
        uartName = name;
    }
}
#define FRAMEHEAD 4
#define PROTOCOLBODY 7
void LcmSlave::uartDataComming(QString name, QByteArray d)
{
    if (uartName != name)  return;
    QByteArray data = d;
    if (data.isEmpty()) return;
    if (data.at(0) == 0x68) {       // new data
        if (!uartBuf.isEmpty())
            if (uartBuf.at(0) != 0x68) uartBuf.clear();
        uartBuf.append(data);
    } else if (!uartBuf.isEmpty()){  // append data
        if (uartBuf.at(0) == 0x68) {
            uartBuf.append(data);
        } else {
            uartBuf.clear();
            return;
        }
    }
    while (uartBuf.size() >= FRAMEHEAD) {
        quint8 size = qint8(uartBuf.at(FRAMEHEAD-1));
        if (uartBuf.size() >= (size+PROTOCOLBODY)) { // length
            QByteArray temp = uartBuf.mid(0, size+PROTOCOLBODY);
            if (temp.size() == size+PROTOCOLBODY) {
                uartBuf.remove(0, size+PROTOCOLBODY);
            } else {
                break;
            }
            if (temp.at(0) == 0x68 && temp.at(temp.size()-1) == 0x7E) { // check head and tail
                QByteArray checkCRC = temp.mid(0, size+FRAMEHEAD);
                quint16 getCRCValue = (quint8(temp.at(size+FRAMEHEAD+1))<<8) +
                                      (quint8(temp.at(size+FRAMEHEAD)));
                if (getCRCValue == Uart::CRCCheck(checkCRC)) {
                    // emit handleMcuData(protocol, quint8(temp.at(1)), temp.mid(2, size+2));
                    handleUartData(quint8(temp.at(1)), temp);
                }
            }
        } else {
            break;
        }
    }
}
void LcmSlave::handleUartData(quint8 , QByteArray d)
{
    quint8 type = d.at(2);
    switch (type) {
    default: break;
    case 0x81:
        emit sendDataToUart(uartName, d);
    break;
    case 0x87: {
        emit sendDataToUart(uartName, d);
        quint8 size = quint8(d.at(FRAMEHEAD-1));
        QByteArray data = d.mid(FRAMEHEAD, size);
        emit updateUserParams(data);
    }
    break;
    case 0x88: { // Get Data
        QByteArray data;
        emit collectChargerData(1, data);
        QByteArray head;
        head.append(char(0x68)).append(char(0)).append(char(0x88)).append(char(sizeof(lcm_slave_mcu_params_t)));
        data.prepend(head);
        quint16 crc = Uart::CRCCheck(data);
        data.append(char(crc&0xff)).append(char(crc>>8));
        emit sendDataToUart(uartName, data);
    }
    break;
    }
}
