#include <Arduino.h>
#include <string.h>

#include "src/ArduinoRTOS.h"

#define LOG_ENTRIES 16
#define LOG_SIZE 32
#define U32_STR_LEN 11

struct LogEntry
{
    char text[LOG_SIZE];
};

volatile LogEntry logQueue[LOG_ENTRIES];
volatile uint8_t logHead = 0;
volatile uint8_t logTail = 0;

volatile uint32_t count1 = 0;
volatile uint32_t count2 = 0;

static void enqueueLog(const char *msg)
{
    uint8_t sreg = SREG;
    cli();

    const uint8_t next = (logHead + 1) % LOG_ENTRIES;

    if (next != logTail)
    {
        strncpy((char *)logQueue[logHead].text, msg, LOG_SIZE - 1);
        logQueue[logHead].text[LOG_SIZE - 1] = '\0';
        logHead = next;
    }

    SREG = sreg;
}

static bool dequeueLog(char *out)
{
    uint8_t sreg = SREG;
    cli();

    if (logHead == logTail)
    {
        SREG = sreg;
        return false;
    }

    strcpy(out, (char *)logQueue[logTail].text);
    logTail = (logTail + 1) % LOG_ENTRIES;
    SREG = sreg;
    return true;
}

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

static void u32toa(uint32_t value, char *buf)
{
    char tmp[10];
    uint8_t i = 0;

    do
    {
        tmp[i++] = '0' + (value % 10);
        value /= 10;
    } while (value);

    while (i)
    {
        *buf++ = tmp[--i];
    }

    *buf = '\0';
}

void task1()
{
    char msg[LOG_SIZE];
    char num[U32_STR_LEN];

    while (1)
    {
        strcpy(msg, "task1 count=");
        u32toa(count1++, num);
        strcat(msg, num);
        enqueueLog(msg);
        busyLoop(50000);
    }
}

void task2()
{
    char msg[LOG_SIZE];
    char num[U32_STR_LEN];

    while (1)
    {
        strcpy(msg, "task2 count=");
        u32toa(count2++, num);
        strcat(msg, num);
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
