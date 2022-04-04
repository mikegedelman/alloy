#include <kernel/cpu.h>
#include <kernel/drivers/uart16550.h>

SerialPort com1;

SerialPort new_serialport(int base) {
    SerialPort s = {
        .data = base,
        .int_en = base + 1,
        .fifo_ctrl = base + 2,
        .line_ctrl = base + 3,
        .modem_ctrl = base + 4,
        .line_sts = base + 5
    };
    // Disable interrupts
    outb(s.int_en, 0x00);

    // Enable DLAB
    outb(s.line_ctrl, 0x80);

    // Set maximum speed to 38400 bps by configuring DLL and DLM
    outb(s.data, 0x03);
    outb(s.int_en, 0x00);

    // Disable DLAB and set data word length to 8 bits
    outb(s.line_ctrl, 0x03);

    // Enable FIFO, clear TX/RX queues and
    // set interrupt watermark at 14 bytes
    outb(s.fifo_ctrl, 0xC7);

    // Mark data terminal ready, signal request to send
    // and enable auxilliary output #2 (used as interrupt line for CPU)
    outb(s.modem_ctrl, 0x0B);

    // Enable interrupts
    outb(s.int_en, 0x01);

    return s;
}

void serial_init() {
    com1 = new_serialport(SERIAL_COM1);
}

int is_transmit_empty(SerialPort *s) {
   return inb(s->line_sts) & 0x20;
}
 
void serial_writeb(SerialPort *s, char a) {
   while (is_transmit_empty(s) == 0);
 
   outb(s->data ,a);
}

int serial_received(SerialPort *s) {
   return inb(s->line_sts) & 1;
}
 
char serial_readb(SerialPort *s) {
   while (serial_received(s) == 0);
 
   return inb(s->data);
}

void serial_write(SerialPort *s, const char *str) {
    int i = 0;
    while (str[i]) {
        serial_writeb(s, str[i]);
        i += 1;
    }
}
