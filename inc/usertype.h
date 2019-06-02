#ifndef USERTYPE_H
#define USERTYPE_H
#include <QSerialPort>

struct uart_param_t {
    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
};
union error_flags_t {
    quint16 all;
    struct {
        quint8 e0:1;
        quint8 e1:1;
        quint8 e2:1;
        quint8 e3:1;
        quint8 e4:1;
        quint8 e5:1;
        quint8 e6:1;
        quint8 e7:1;
        quint8 e8:1;
        quint8 e9:1;
        quint8 e10:1;
        quint8 e11:1;
        quint8 e12:1;
        quint8 e13:1;
        quint8 e14:1;
        quint8 e15:1;
    }bits;
};
#pragma pack(1)
typedef struct {
    quint16 rate_vol;
    quint16 rate_cur;
    quint16 oc_ratio;
    quint16 ov_ratio;
    quint16 lv_ratio;
    quint16 ch_threshold;
    quint16 ev_threshold;
    quint8 drop_threshold;
    quint8 pwm;
    quint8  protected_time;
    quint16 min_cur;
}  __attribute__((packed, aligned(1))) lcm_slave_mcu_params_t;
#pragma pack(1)
typedef struct {
    quint8 charger_state;
    quint8 plug_state;
    quint32 vol;
    quint32 cur;
    quint32 energy;
    quint8  time[3];
    quint8  stop_reason;
    quint8  falut_reason;
} lcm_slave_mcu_charging_data_t;

#endif // USERTYPE_H
