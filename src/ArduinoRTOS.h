#ifndef ARDUINO_RTOS_H
#define ARDUINO_RTOS_H

#include <Arduino.h>

namespace ArduinoRTOS
{
using TaskEntry = void (*)(void);

static constexpr uint8_t MAX_TASKS = 3;
static constexpr uint8_t STACK_SIZE = 128;

bool createTask(TaskEntry entry);
void start();
uint8_t taskCount();
}

#endif
