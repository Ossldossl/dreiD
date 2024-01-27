#pragma once
#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define GLEW_STATIC

#define for_in(identifier, init, max, inc) for (int (identifier) = init; (identifier) < max; (identifier)+=(inc))
#define for_to(identifier, max) for (int (identifier) = 0; (identifier) < max; (identifier)+=1)
#define key_pressed(ev, key) (ev.key_code == (key) && ev.pressed == true)
#define null NULL
#define DD_MAX_KEYEVENT_COUNT 10

typedef struct {
    HWND hWnd;
    HGLRC rc;
    HDC dc;
    uint16_t width, height;
    char* title;
} dd_window_t;

typedef struct {
    char key_code;
    bool pressed; // true => pressed; false => released
} dd_key_event;

typedef struct {
    float dt;
    dd_window_t* primary_window;
    dd_window_t** windows;
    uint16_t window_count; // excluding primary window
    LARGE_INTEGER pf_freq;
    struct {
        bool alt, shift, ctrl;
    } mod_keys;
    dd_key_event key_events[DD_MAX_KEYEVENT_COUNT];
    uint8_t event_count;
    uint32_t cur_sp;
    uint64_t start_time;
} dd_ctx;

typedef void (*dd_app_lifecycle_callback)(dd_ctx*);

typedef struct {
    uint16_t width, height;
    char* title;
    dd_app_lifecycle_callback setup, frame, destroy;
    bool is_reactive;
} dd_app_desc;

int dd_app(dd_app_desc* app_desc); 
// time in milliseconds after startup (main-window creation)
double dd_time(dd_ctx* ctx);
char* dd_read_file(char* file_name, uint32_t* file_size);