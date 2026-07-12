#include "ArduinoRTOS.h"

#include <avr/interrupt.h>

namespace ArduinoRTOS
{
struct TCB
{
    uint8_t *sp;
    uint8_t stack[STACK_SIZE];
};

static TCB tasks[MAX_TASKS];
static volatile uint8_t currentTask = 0;
static volatile uint8_t createdTasks = 0;

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
    if (createdTasks > 1)
    {
        currentTask = (currentTask + 1) % createdTasks;
    }
}

bool createTask(TaskEntry entry)
{
    if (createdTasks >= MAX_TASKS || entry == nullptr)
    {
        return false;
    }

    const uint8_t id = createdTasks;
    uint8_t *sp = &tasks[id].stack[STACK_SIZE - 1];
    const uint16_t pc = (uint16_t)entry;

    *sp-- = pc & 0xFF;
    *sp-- = pc >> 8;

    *sp-- = 0x00;
    *sp-- = 0x80;

    for (int reg = 1; reg <= 31; reg++)
    {
        *sp-- = 0x00;
    }

    tasks[id].sp = sp + 1;
    createdTasks++;
    return true;
}

uint8_t taskCount()
{
    return createdTasks;
}

ISR(TIMER1_COMPA_vect, ISR_NAKED)
{
    SAVE_CONTEXT();

    tasks[currentTask].sp = (uint8_t *)SP;

    scheduler();

    SP = (uint16_t)tasks[currentTask].sp;

    RESTORE_CONTEXT();
}

void start()
{
    if (createdTasks == 0)
    {
        return;
    }

    cli();

    TCCR1A = 0;
    TCCR1B = 0;

    OCR1A = 249;

    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11) | (1 << CS10);
    TIMSK1 |= (1 << OCIE1A);

    currentTask = 0;
    SP = (uint16_t)tasks[0].sp;

    sei();

    RESTORE_CONTEXT();
}
}
