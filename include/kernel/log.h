#pragma once

#include <kernel/drivers/uart16550.h>

// TODO implement more logging stuff

#define LOG_INFO

#ifdef LOG_INFO
#define INFO(x) serial_write(&com1, x)
#else
#define INFO(x) do { } while(0)
#endif