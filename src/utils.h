#pragma once

#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#ifdef __ANDROID__

#include <android/log.h>
#define TAG "NES_EMULATOR"
#define PRINTF(FMT, ...) __android_log_print(ANDROID_LOG_DEBUG, FMT, TAG, __VA_ARGS__)

#else

#define PRINTF(...) printf(__VA_ARGS__)

#endif

#define LOGLEVEL 1
#define TRACER 0
#define PROFILE 0
#define PROFILE_STOP_FRAME 1800

enum {
    BIT_7 = 1<<7,
    BIT_6 = 1<<6,
    BIT_5 = 1<<5,
    BIT_4 = 1<<4,
    BIT_3 = 1<<3,
    BIT_2 = 1<<2,
    BIT_1 = 1<<1,
    BIT_0 = 1
};

enum LogLevel{
    DEBUG = 0,
    ERROR,
    INFO,
};

size_t file_size(FILE* file);
void LOG(enum LogLevel logLevel, const char* fmt, ...);
void TRACE(const char* fmt, ...);

// midpoint circle algorithm rendering utils
int SDL_RenderDrawCircle(SDL_Renderer * renderer, int x, int y, int radius);
int SDL_RenderFillCircle(SDL_Renderer * renderer, int x, int y, int radius);