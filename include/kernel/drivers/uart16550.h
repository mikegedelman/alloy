#pragma once

#define SERIAL_COM1 0x3F8


typedef struct SerialPortTag {
    int data;
    int int_en;
    int fifo_ctrl;
    int line_ctrl;
    int modem_ctrl;
    int line_sts;
} SerialPort;

extern SerialPort com1;

SerialPort new_serialport(int base_port);
void serial_write(SerialPort*, const char*);
void serial_writeb(SerialPort*, char a);
char serial_read(SerialPort*);
void serial_init();
