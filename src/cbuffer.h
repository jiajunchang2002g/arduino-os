#ifndef CBUFFER_H
#define CBUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define LOG_ENTRIES 16
#define LOG_SIZE 32

void enqueueLog(const char *msg);
bool dequeueLog(char *out);

#endif
