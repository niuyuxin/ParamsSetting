#include "mainwindow.h"
#include <QLabel>
#include <QDebug>
#include <QStatusBar>
#include <QDateTime>
#include "usertype.h"


MainWindow::MainWindow()
{
    qRegisterMetaType<uart_param_t>("uart_param_t");
    QLabel *sbl = new QLabel("Version 2019.01.03");
    statusBar()->addWidget(sbl);
    statusBar()->addWidget(&statusTips, 1);
    statusTips.setText("hello, world!");
    centralFrame = new QFrame(this);
    frameUi.setupUi(centralFrame);
    frameUi.baudRate->setCurrentText("9600");
    setCentralWidget(centralFrame);
    uart = new Uart();
    uartThread = new QThread();
    uart->moveToThread(uartThread);
    uartThread->start();
    connect(uart, &Uart::newUartPlugin, this, &MainWindow::onNewUartPlugin);
    connect(this, &MainWindow::openUart, uart, &Uart::openXUart);
    connect(this, &MainWindow::closeUart, uart, &Uart::closeXUart);
    connect(uart, &Uart::logMessage, this, &MainWindow::onLogMessage);
    connect(this, &MainWindow::sendData, uart, &Uart::onSendData);
    connect(frameUi.uartButton, &QPushButton::toggled, this, &MainWindow::onUartButtonClicked);
    connect(frameUi.applyParamButton, &QPushButton::clicked, this, &MainWindow::onApplyParamButton);
    connect(frameUi.lcmUartButton, &QPushButton::toggled, this, &MainWindow::onLcmUartButtonToggle);
    lcmSlave = new LcmSlave();
    lcmSlaveThread = new QThread();
    lcmSlave->moveToThread(lcmSlaveThread);
    lcmSlaveThread->start();
    qRegisterMetaType<QString>("QString");
    QObject::connect(lcmSlave, &LcmSlave::sendDataToUart, uart, &Uart::onSendData);
    QObject::connect(this, &MainWindow::allocateUart, lcmSlave, &LcmSlave::onGetUartName);
    QObject::connect(lcmSlave, &LcmSlave::collectChargerData, this, &MainWindow::onCollectChargerData, Qt::DirectConnection);
    QObject::connect(uart, &Uart::newDataComming, lcmSlave, &LcmSlave::uartDataComming);
    // connect(qApp, &QApplication::focusChanged, this, &MainWindow::onFocusChanged);
    initACChargerParams();
}
MainWindow::~MainWindow()
{
    delete uart;
}
void MainWindow::initACChargerParams()
{
    frameUi.rateVolLineEdit->setText(QString::number(220.1));
    frameUi.rateCurLineEdit->setText(QString::number(30.1));
    frameUi.ocRatioLineEdit->setText(QString::number(1.11));
    frameUi.ovRatioLineEdit->setText(QString::number(1.2));
    frameUi.lvRatioLineEdit->setText(QString::number(1.3));
    frameUi.plugThresholdLineEdit->setText(QString::number(10));
    frameUi.evThresholdLineEdit->setText(QString::number(12));
    frameUi.dropThresholdLineEdit->setText(QString::number(25));
    frameUi.pwmLineEdit->setText(QString::number(50));
    frameUi.protectedTimeLineEdit->setText(QString::number(100));
    frameUi.minCurLineEdit->setText(QString::number(2.3));
}

void MainWindow::onFocusChanged(QWidget* old, QWidget* now)
{
    qDebug() << (now != NULL ? now->objectName() : 0 ) << (old != NULL ? old->objectName() : 0);
}
void MainWindow::onNewUartPlugin(QStringList name, QStringList manufacturer)
{
    frameUi.uartName->clear();
    for (int i = 0; i < name.size(); i++) {
        frameUi.uartName->insertItem(0, name.at(i) + "-" + manufacturer.at(i));
        frameUi.lcmUartCombo->insertItem(0, name.at(i) + "-" + manufacturer.at(i));
        if (manufacturer.at(i) == "Prolific" && !frameUi.uartName->currentText().contains("Prolific")) {
            frameUi.uartName->setCurrentText(name.at(i) + "-" + manufacturer.at(i));
            frameUi.lcmUartCombo->setCurrentText(name.at(i) + "-" + manufacturer.at(i));
        }
    }
    statusBar()->showMessage("uart changed", 1500);
}
void MainWindow::onUartButtonClicked(bool toggle)
{
    if (toggle) {
        QString parityStr = frameUi.parity->currentText();
        QSerialPort::Parity p = parityStr == "Even" ? QSerialPort::EvenParity :
                                parityStr == "Odd" ? QSerialPort::OddParity : QSerialPort::NoParity;

        uart_param_t temp = {QSerialPort::BaudRate(frameUi.baudRate->currentText().toUInt()),
                                   QSerialPort::DataBits(frameUi.dataBits->currentText().toUInt()),
                                   p,
                                   QSerialPort::StopBits(frameUi.stopBits->currentText().toUInt())
                                  };
        uart1 = frameUi.uartName->currentText();
        emit openUart(frameUi.uartName->currentText(), temp);
    } else {
        emit closeUart(uart1);
    }
}
void MainWindow::onLcmUartButtonToggle(bool t)
{
    if (t) {
        QString parityStr = frameUi.lcmParityCombo->currentText();
        QSerialPort::Parity p = parityStr == "Even" ? QSerialPort::EvenParity :
                                parityStr == "Odd" ? QSerialPort::OddParity : QSerialPort::NoParity;

        uart_param_t temp = {QSerialPort::BaudRate(frameUi.lcmBaudRateCombo->currentText().toUInt()),
                                   QSerialPort::DataBits(frameUi.lcmDataBitsCombo->currentText().toUInt()),
                                   p,
                                   QSerialPort::StopBits(frameUi.lcmStopBitsCombo->currentText().toUInt())
                                  };
        uart2 = frameUi.lcmUartCombo->currentText();
        emit allocateUart(lcmSlave->objectName(), uart2);
        emit openUart(frameUi.lcmUartCombo->currentText(), temp);
    } else {
        emit closeUart(uart2);
    }
}

void MainWindow::onLogMessage(QString m)
{
    frameUi.textBrowser->insertPlainText(m.prepend(QDateTime::currentDateTime().toString("[yy.MM.dd hh:mm:ss.zzz] ")));
    frameUi.textBrowser->insertPlainText("\r\n");
    frameUi.textBrowser->moveCursor(QTextCursor::EndOfLine);
}
void MainWindow::onApplyParamButton()
{
    // error flags
    error_flags_t ef;
    ef.all = 0;
    ef.bits.e0 = frameUi.checkBoxE0->isChecked();
    ef.bits.e1 = frameUi.checkBoxE1->isChecked();
    ef.bits.e2 = frameUi.checkBoxE2->isChecked();
    ef.bits.e3 = frameUi.checkBoxE3->isChecked();
    ef.bits.e4 = frameUi.checkBoxE4->isChecked();
    ef.bits.e5 = frameUi.checkBoxE5->isChecked();
    ef.bits.e6 = frameUi.checkBoxE6->isChecked();
    ef.bits.e7 = frameUi.checkBoxE7->isChecked();
    ef.bits.e8 = frameUi.checkBoxE8->isChecked();
    ef.bits.e9 = frameUi.checkBoxE9->isChecked();
    ef.bits.e10 = frameUi.checkBoxE10->isChecked();
    ef.bits.e11 = frameUi.checkBoxE11->isChecked();
    ef.bits.e12 = frameUi.checkBoxE12->isChecked();
    QByteArray d((char*)(&ef), 2);
    QByteArray temp;
    temp.append(0x68).append(5).append(quint8(d.size())).append(d);
    temp.insert(1, 1);
    quint16 crc = Uart::CRCCheck(temp);
    temp.append(quint8(crc&0xff)).append(quint8(crc>>8));
    temp.append(0x7e);
    emit sendData(uart1, temp);
}

void MainWindow::onCollectChargerData(quint8 type, QByteArray &d)
{
    if (type == 0) {
        lcm_slave_mcu_charging_data_t chargerData[2];
        chargerData[0].charger_state = frameUi.chargerStateCombo->currentIndex();
        chargerData[0].plug_state = frameUi.plugStateCombo->currentIndex();
        chargerData[0].vol = frameUi.chargingVolLineEdit->text().toFloat()*10;
        chargerData[0].cur = frameUi.chargingCurLineEdit->text().toFloat()*10;
        chargerData[0].energy = frameUi.chargingEnergyLineEdit->text().toFloat()*10;
        chargerData[0].time[0] = frameUi.chargingTimeEdit->dateTime().time().hour();
        chargerData[0].time[1] = frameUi.chargingTimeEdit->dateTime().time().minute();
        chargerData[0].time[2] = frameUi.chargingTimeEdit->dateTime().time().second();
        chargerData[0].stop_reason = frameUi.chargingStopReasonCombo->currentIndex();
        chargerData[0].falut_reason = frameUi.chargingFaultReasonCombo->currentIndex();

        chargerData[1].charger_state = frameUi.chargerStateCombo_2->currentIndex();
        chargerData[1].plug_state = frameUi.plugStateCombo_2->currentIndex();
        chargerData[1].vol = frameUi.chargingVolLineEdit_2->text().toFloat()*10;
        chargerData[1].cur = frameUi.chargingCurLineEdit_2->text().toFloat()*10;
        chargerData[1].energy = frameUi.chargingEnergyLineEdit_2->text().toFloat()*10;
        chargerData[1].time[0] = frameUi.chargingTimeEdit_2->dateTime().time().hour();
        chargerData[1].time[1] = frameUi.chargingTimeEdit_2->dateTime().time().minute();
        chargerData[1].time[2] = frameUi.chargingTimeEdit_2->dateTime().time().second();
        chargerData[1].stop_reason = frameUi.chargingStopReasonCombo_2->currentIndex();
        chargerData[1].falut_reason = frameUi.chargingFaultReasonCombo_2->currentIndex();
        d.clear();
        d.append((char*)&chargerData, sizeof(chargerData));
    } else if (type == 1) {
        lcm_slave_mcu_params_t params;
        params.rate_vol = frameUi.rateVolLineEdit->text().toFloat()*10;
        params.rate_cur = frameUi.rateCurLineEdit->text().toFloat() *10;
        params.oc_ratio = frameUi.ocRatioLineEdit->text().toFloat()*10;
        params.ov_ratio = frameUi.ovRatioLineEdit->text().toFloat()*10;
        params.lv_ratio = frameUi.lvRatioLineEdit->text().toFloat()*10;
        params.ev_threshold = frameUi.evThresholdLineEdit->text().toUInt();
        params.ch_threshold = frameUi.plugThresholdLineEdit->text().toUInt();
        params.drop_threshold = frameUi.dropThresholdLineEdit->text().toUInt();
        params.pwm = frameUi.pwmLineEdit->text().toUInt();
        params.protected_time = frameUi.protectedTimeLineEdit->text().toUInt();
        params.min_cur = frameUi.minCurLineEdit->text().toUInt();
        d.clear();
        d.append((char*)&params, sizeof(params));
    }
}

void MainWindow::onUpdateUserParams(QByteArray data)
{
    statusBar()->showMessage("User Set Parameters!", 2000);
    lcm_slave_mcu_params_t params;
    memset(&params, 0, sizeof(params));
    memcpy(&params, data.data(), data.size());
    frameUi.rateVolLineEdit->setText(QString::number(params.rate_vol/10.0, 'f', 1));
    frameUi.rateCurLineEdit->setText(QString::number(params.rate_cur/10.0, 'f', 1));
    frameUi.ocRatioLineEdit->setText(QString::number(params.oc_ratio/10.0, 'f', 1));
    frameUi.ovRatioLineEdit->setText(QString::number(params.ov_ratio/10.0, 'f', 1));
    frameUi.lvRatioLineEdit->setText(QString::number(params.lv_ratio/10.0, 'f', 1));
    frameUi.plugThresholdLineEdit->setText(QString::number(params.ev_threshold));
    frameUi.evThresholdLineEdit->setText(QString::number(params.ev_threshold));
    frameUi.dropThresholdLineEdit->setText(QString::number(params.drop_threshold));
    frameUi.pwmLineEdit->setText(QString::number(params.pwm));
    frameUi.protectedTimeLineEdit->setText(QString::number(params.protected_time));
    frameUi.minCurLineEdit->setText(QString::number(params.min_cur/10.0, 'f', 1));
}
