# Arduino RTOS (Minimal)

This repository now exposes a tiny cooperative/preemptive-style RTOS core as an Arduino library.

## Structure

- `/home/runner/work/arduino-os/arduino-os/src/ArduinoRTOS.h` - public API
- `/home/runner/work/arduino-os/arduino-os/src/ArduinoRTOS.cpp` - scheduler/context switching implementation
- `/home/runner/work/arduino-os/arduino-os/main.ino` - simple usage example with 3 tasks

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
