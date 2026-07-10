#include <Arduino.h>
#include <avr/interrupt.h>

#define NUM_TASKS   2
#define STACK_SIZE  128
#define BAUD_RATE 115200
#define U32_STR_LEN 11

typedef struct {
    uint8_t *sp;
    uint8_t stack[STACK_SIZE];
} TCB;

TCB tasks[NUM_TASKS];
volatile uint8_t current_task = 0;

#define SAVE_CONTEXT()                     \
    asm volatile(                          \
        "push r0            \n\t"          \
        "in   r0, __SREG__  \n\t"          \
        "push r0            \n\t"          \
        "clr  r1            \n\t"          \
        "push r1            \n\t"          \
        "push r2            \n\t"          \
        "push r3            \n\t"          \
        "push r4            \n\t"          \
        "push r5            \n\t"          \
        "push r6            \n\t"          \
        "push r7            \n\t"          \
        "push r8            \n\t"          \
        "push r9            \n\t"          \
        "push r10           \n\t"          \
        "push r11           \n\t"          \
        "push r12           \n\t"          \
        "push r13           \n\t"          \
        "push r14           \n\t"          \
        "push r15           \n\t"          \
        "push r16           \n\t"          \
        "push r17           \n\t"          \
        "push r18           \n\t"          \
        "push r19           \n\t"          \
        "push r20           \n\t"          \
        "push r21           \n\t"          \
        "push r22           \n\t"          \
        "push r23           \n\t"          \
        "push r24           \n\t"          \
        "push r25           \n\t"          \
        "push r26           \n\t"          \
        "push r27           \n\t"          \
        "push r28           \n\t"          \
        "push r29           \n\t"          \
        "push r30           \n\t"          \
        "push r31           \n\t"          \
    )

#define RESTORE_CONTEXT()                  \
    asm volatile(                          \
        "pop r31            \n\t"          \
        "pop r30            \n\t"          \
        "pop r29            \n\t"          \
        "pop r28            \n\t"          \
        "pop r27            \n\t"          \
        "pop r26            \n\t"          \
        "pop r25            \n\t"          \
        "pop r24            \n\t"          \
        "pop r23            \n\t"          \
        "pop r22            \n\t"          \
        "pop r21            \n\t"          \
        "pop r20            \n\t"          \
        "pop r19            \n\t"          \
        "pop r18            \n\t"          \
        "pop r17            \n\t"          \
        "pop r16            \n\t"          \
        "pop r15            \n\t"          \
        "pop r14            \n\t"          \
        "pop r13            \n\t"          \
        "pop r12            \n\t"          \
        "pop r11            \n\t"          \
        "pop r10            \n\t"          \
        "pop r9             \n\t"          \
        "pop r8             \n\t"          \
        "pop r7             \n\t"          \
        "pop r6             \n\t"          \
        "pop r5             \n\t"          \
        "pop r4             \n\t"          \
        "pop r3             \n\t"          \
        "pop r2             \n\t"          \
        "pop r1             \n\t"          \
        "pop r0             \n\t"          \
        "out __SREG__, r0   \n\t"          \
        "pop r0             \n\t"          \
        "clr r1             \n\t"          \
        "reti               \n\t"          \
    )

static inline void scheduler()
{
    current_task = (current_task + 1) % NUM_TASKS;
}

void create_task(uint8_t id, void (*entry)(void))
{
    uint8_t *sp = &tasks[id].stack[STACK_SIZE - 1];

    uint16_t pc = (uint16_t)entry;

    /*
     * Bottom of interrupt frame:
     * RETI will consume these last.
     */
    *sp-- = pc & 0xFF;      // PC low
    *sp-- = pc >> 8;        // PC high

    /*
     * SAVE_CONTEXT pushed:
     *
     * push r0
     * in r0,SREG
     * push r0
     * push r1
     * ...
     * push r31
     *
     * Therefore RESTORE_CONTEXT expects:
     *
     * pop r31
     * ...
     * pop r1
     * pop r0 -> SREG
     * out SREG,r0
     * pop r0 -> original r0
     * reti
     */

    *sp-- = 0x00;           // original r0
    *sp-- = 0x80;           // SREG with I-bit set

    for (int reg = 1; reg <= 31; reg++)
        *sp-- = 0x00;       // r1-r31

    tasks[id].sp = sp + 1;
}

ISR(TIMER1_COMPA_vect, ISR_NAKED)
{
    SAVE_CONTEXT();

    tasks[current_task].sp = (uint8_t *)SP;

    scheduler();

    SP = (uint16_t)tasks[current_task].sp;

    RESTORE_CONTEXT();
}

void start_scheduler()
{
    cli();

    TCCR1A = 0;
    TCCR1B = 0;

    OCR1A = 249;               // 1ms @16MHz prescaler 64

    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11) | (1 << CS10);

    TIMSK1 |= (1 << OCIE1A);

    current_task = 0;
    SP = (uint16_t)tasks[0].sp;

    sei();

    RESTORE_CONTEXT();
}

#define LOG_ENTRIES 16
#define LOG_SIZE    32

struct LogEntry {
    char text[LOG_SIZE];
};

volatile LogEntry log_queue[LOG_ENTRIES];
volatile uint8_t log_head = 0;
volatile uint8_t log_tail = 0;

volatile uint32_t count1 = 0;
volatile uint32_t count2 = 0;

void enqueue_log(const char *msg)
{
    uint8_t sreg = SREG;
    cli();

    uint8_t next = (log_head + 1) % LOG_ENTRIES;

    if (next != log_tail)
    {
        strncpy(
            (char *)log_queue[log_head].text,
            msg,
            LOG_SIZE - 1
        );

        log_queue[log_head].text[LOG_SIZE - 1] = '\0';

        log_head = next;
    }

    SREG = sreg;
}

bool dequeue_log(char *out)
{
    uint8_t sreg = SREG;
    cli();

    if (log_head == log_tail)
    {
        SREG = sreg;
        return false;
    }

    strcpy(
        out,
        (char *)log_queue[log_tail].text
    );

    log_tail = (log_tail + 1) % LOG_ENTRIES;

    SREG = sreg;

    return true;
}

void busy_loop(uint32_t count)
{
    for (volatile uint32_t i = 0; i < count; i++)
    {
        asm volatile("nop");
    }
}

static void uart_init(void)
{
    UBRR0H = 0;
    UBRR0L = 8;                  // 115200 baud @ 16MHz

    UCSR0A = 0;
    UCSR0B = (1 << TXEN0);       // TX enable
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1
}

static void uart_putc(char c)
{
    while (!(UCSR0A & (1 << UDRE0)))
        ;

    UDR0 = c;
}

static void uart_puts(const char *s)
{
    while (*s)
        uart_putc(*s++);
}

static void u32toa(uint32_t v, char *buf)
{
    char tmp[10];
    uint8_t i = 0;

    do
    {
        tmp[i++] = '0' + (v % 10);
        v /= 10;
    }
    while (v);

    while (i)
        *buf++ = tmp[--i];

    *buf = '\0';
}

void task1(void)
{
    char msg[LOG_SIZE];
    char num[U32_STR_LEN];

    while (1)
    {
        strcpy(msg, "task1 count=");
        u32toa(count1++, num);
        strcat(msg, num);

        enqueue_log(msg);

        busy_loop(50000);
    }
}

void task1(void)
{
    char msg[LOG_SIZE];
    char num[U32_STR_LEN];

    while (1)
    {
        strcpy(msg, "task2 count=");
        u32toa(count2++, num);
        strcat(msg, num);

        enqueue_log(msg);

        busy_loop(50000);
    }
}

void printer_task(void)
{
    char msg[LOG_SIZE];

    while (1)
    {
        if (dequeue_log(msg))
        {
            uart_puts(msg);
            uart_puts("\r\n");
        }
        busy_loop(1000);
    }
}

void setup()
{
    uart_init();

    create_task(0, task1);
    create_task(1, task2);
    create_task(2, printer_task);

    start_scheduler();

    while (1)
    {
    }
}

void loop()
{
}
