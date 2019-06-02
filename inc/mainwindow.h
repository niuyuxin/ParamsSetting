#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <ui_mainwindow.h>
#include <QLabel>
#include "uart.h"
#include "lcmslave.h"
#include <QThread>

class MainWindow: public QMainWindow
{
    Q_OBJECT
public:
     MainWindow();
    ~MainWindow();
private:
    QLabel statusTips;
    Uart* uart;
    QThread *uartThread;
    QFrame* centralFrame;
    Ui::Frame frameUi;
    QString uart1;
    QString uart2;
    LcmSlave *lcmSlave;
    QThread *lcmSlaveThread;
    void initACChargerParams();
signals:
    openUart(QString name,  uart_param_t);
    closeUart(QString name);
    sendData(QString, QByteArray);
    allocateUart(QString who, QString name);
public slots:
    void onFocusChanged(QWidget* n, QWidget* old);
    void onNewUartPlugin(QStringList, QStringList);
    void onUartButtonClicked(bool);
    void onLogMessage(QString m);
    void onApplyParamButton();
    void onLcmUartButtonToggle(bool t);
    void onCollectChargerData(quint8, QByteArray&);
    void onUpdateUserParams(QByteArray);
};

#endif // MAINWINDOW_H
