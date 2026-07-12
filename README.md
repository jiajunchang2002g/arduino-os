# Arduino RTOS

This is an Arduino library for a tiny cooperative/preemptive-style Real-Time Operating System (RTOS) !

## Structure

- `src/ArduinoRTOS.h` - public API
- `src/ArduinoRTOS.cpp` - scheduler/context switching implementation
- `main.ino` - simple usage example with 3 tasks

## API

Include the library:

```cpp
#include "src/ArduinoRTOS.h"
```

Create task functions with signature:

```cpp
void myTask();
```

Register tasks in `setup()` and then start the scheduler:

```cpp
ArduinoRTOS::createTask(task1);
ArduinoRTOS::createTask(task2);
ArduinoRTOS::start();
```

## Notes
- `ArduinoRTOS::MAX_TASKS` is currently `3`.
- Each task gets `ArduinoRTOS::STACK_SIZE` bytes of stack. 
- Tasks must run forever (typically `while(1)` loops).
- Example output is written through UART at 115200 baud.

## Spec
- Arduin UNO R3 board with Atmega328p processor (2Kb SRAM)

## References
- https://docs.arduino.cc/hardware/uno-rev3/#features
- https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf


