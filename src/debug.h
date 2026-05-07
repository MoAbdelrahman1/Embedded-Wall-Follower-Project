#pragma once
#include <stdint.h>
#include <stdarg.h>

void Debug_Init(uint32_t baud);
void Debug_Printf(const char* format, ...);

#define DBG_BEGIN(baud)   Debug_Init(baud)
#define DBG(msg)          Debug_Printf("%s", msg)
#define DBGLN(msg)        Debug_Printf("%s\n", msg)
#define DBGF(...)         Debug_Printf(__VA_ARGS__)
