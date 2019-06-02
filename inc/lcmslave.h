#ifndef LCMSLAVE_H
#define LCMSLAVE_H
#include <QObject>
#include <QTimer>

class LcmSlave: public QObject
{
    Q_OBJECT
public:
    LcmSlave(QObject *p = 0):QObject(p){
        setObjectName("LcmSlave");
        cycleSendUartDataTimer = new QTimer(this);
        cycleSendUartDataTimer->setInterval(500);
        QObject::connect(cycleSendUartDataTimer, &QTimer::timeout, this, &LcmSlave::onCycleSendUartDataTimerTimeout);
        cycleSendUartDataTimer->start();
    }
    ~LcmSlave();
private:
    QString uartName;
    QByteArray uartBuf;
    QTimer *cycleSendUartDataTimer;
    void handleUartData(quint8, QByteArray);
public slots:
    void onGetUartName(QString who, QString);
    void getDataFromUart(QString, QByteArray);
    void onCycleSendUartDataTimerTimeout();
    void uartDataComming(QString, QByteArray);
signals:
    void updateUserParams(QByteArray);
    void sendDataToUart(QString, QByteArray);
    void collectChargerData(quint8, QByteArray&);
};

#endif // LCMSLAVE_H
