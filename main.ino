#include <Arduino.h>
#include <stdio.h>

#include "src/ArduinoRTOS.h"
#include "src/cbuffer.h"

volatile uint32_t count1 = 0;
volatile uint32_t count2 = 0;

static void busyLoop(uint32_t count)
{
    for (volatile uint32_t i = 0; i < count; i++)
    {
        asm volatile("nop");
    }
}

static void uartInit()
{
    UBRR0H = 0;
    UBRR0L = 8;
    UCSR0A = 0;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static void uartPutc(char c)
{
    while (!(UCSR0A & (1 << UDRE0)))
    {
    }
    UDR0 = c;
}

static void uartPuts(const char *s)
{
    while (*s)
    {
        uartPutc(*s++);
    }
}

void task1()
{
    char msg[LOG_SIZE];

    while (1)
    {
        snprintf(msg, LOG_SIZE, "task1 count=%lu", (unsigned long)count1++);
        enqueueLog(msg);
        busyLoop(50000);
    }
}

void task2()
{
    char msg[LOG_SIZE];

    while (1)
    {
        snprintf(msg, LOG_SIZE, "task2 count=%lu", (unsigned long)count2++);
        enqueueLog(msg);
        busyLoop(50000);
    }
}

void printerTask()
{
    char msg[LOG_SIZE];

    while (1)
    {
        if (dequeueLog(msg))
        {
            uartPuts(msg);
            uartPuts("\r\n");
        }

        busyLoop(1000);
    }
}

void setup()
{
    uartInit();

    ArduinoRTOS::createTask(task1);
    ArduinoRTOS::createTask(task2);
    ArduinoRTOS::createTask(printerTask);

    ArduinoRTOS::start();
}

void loop()
{
}
