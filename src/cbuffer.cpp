#include "cbuffer.h"

#include <string.h>
#include <avr/interrupt.h>

struct LogEntry
{
    char text[LOG_SIZE];
};

static volatile LogEntry logQueue[LOG_ENTRIES];
static volatile uint8_t logHead = 0;
static volatile uint8_t logTail = 0;

void enqueueLog(const char *msg)
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

bool dequeueLog(char *out)
{
    uint8_t sreg = SREG;
    cli();

    if (logHead == logTail)
    {
        SREG = sreg;
        return false;
    }

    strncpy(out, (char *)logQueue[logTail].text, LOG_SIZE - 1);
    out[LOG_SIZE - 1] = '\0';
    logTail = (logTail + 1) % LOG_ENTRIES;
    SREG = sreg;
    return true;
}
