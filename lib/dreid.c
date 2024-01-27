#include <stdlib.h>
#include "dreid.h"
#include "dd_log.h"
#include "dd_gfx.h"

#define null NULL

static LRESULT CALLBACK _dd_wndproc(HWND hWnd, uint32_t msg, WPARAM wParam, LPARAM lParam)
{
    dd_ctx* ctx = (dd_ctx*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    switch (msg) {
        case WM_SIZE: {
            int width = LOWORD(lParam); int height = HIWORD(lParam);
            glViewport(0, 0, width, height);
            break;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps); EndPaint(hWnd, &ps);
            return 0;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_SHIFT) { ctx->mod_keys.shift = true; }
            else if (wParam == VK_CONTROL) { ctx->mod_keys.ctrl = true; }
            else if (wParam == VK_MENU) { ctx->mod_keys.alt = true; }

            ctx->event_count++; if (ctx->event_count > DD_MAX_KEYEVENT_COUNT) {
                log_fatal("To many keyevents for buffer!"); exit(-1);
            }
            ctx->key_events[ctx->event_count-1] = (dd_key_event) {.key_code = wParam, .pressed = true };
            break;
        }
        case WM_KEYUP: {
            if (wParam == VK_SHIFT) { ctx->mod_keys.shift = false; }
            else if (wParam == VK_CONTROL) { ctx->mod_keys.ctrl = false; }
            else if (wParam == VK_MENU) { ctx->mod_keys.alt = false; }
            ctx->event_count++; if (ctx->event_count > DD_MAX_KEYEVENT_COUNT) {
                log_fatal("To many keyevents for buffer!"); exit(-1);
            }
            ctx->key_events[ctx->event_count-1] = (dd_key_event) {.key_code = wParam, .pressed = false };
            break;
        }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int dd_app(dd_app_desc* app_desc)
{
    init_console();
    if (app_desc == null) { return 0; }
    app_desc->width = app_desc->width ? app_desc->width : 1000;
    app_desc->height = app_desc->height ? app_desc->height : 1000;
    
    HINSTANCE hInstance = GetModuleHandle(null);
    const char* class_name = "dreid window class";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = _dd_wndproc;
    wc.hInstance = hInstance;
    wc.lpszClassName = class_name;
    wc.style = CS_OWNDC;
    RegisterClass(&wc);

    HWND hWnd = CreateWindowEx(
        0,
        class_name,
        app_desc->title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 
        app_desc->width, app_desc->height,
        null, null,
        hInstance,
        null
    );
    if (hWnd == null) {
        log_error("Failed to create window: %lu", GetLastError());
        return -1;
    }

    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        32, // colordepth framebuffer
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        24, // depth buffer bits
        8, // stencil buffer bits
        0, // aux buffers
        PFD_MAIN_PLANE,
        0, 0, 0, 0 
    };

    HDC dc = GetDC(hWnd);
    int pixel_format = ChoosePixelFormat(dc, &pfd);
    if (pixel_format == 0) {
        log_error("Failed to find suitable pixelformat: %lu", GetLastError()); return -1;
    }
    SetPixelFormat(dc, pixel_format, &pfd);

    HGLRC rc = wglCreateContext(dc);
    wglMakeCurrent(dc, rc);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        log_error("Failed to initialize Glew: %s", glewGetErrorString(err));
    }

    log_info("OPENGL Version: %s", glGetString(GL_VERSION));
    ShowWindow(hWnd, SW_NORMAL);

    dd_ctx ctx = {0};
    ctx.dt = 0.f;
    QueryPerformanceFrequency(&ctx.pf_freq);
    ctx.primary_window = &(dd_window_t) {
        .hWnd = hWnd,
        .rc = rc,
        .dc = dc,
        .width = app_desc->width,
        .height = app_desc->height,
        .title = app_desc->title,
    };
    LARGE_INTEGER start_time; QueryPerformanceFrequency(&start_time);
    ctx.start_time = start_time.QuadPart;
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (long long)&ctx);

    app_desc->setup(&ctx);
    // main loop
    MSG msg;
    if (app_desc->is_reactive) {
        while (GetMessage(&msg, hWnd, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            app_desc->frame(&ctx);
            ctx.event_count = 0;
        }
    } else {
        LARGE_INTEGER cur_ticks;
        LARGE_INTEGER last_ticks;
        while (true) {
            while (PeekMessage(&msg, null, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (msg.message == WM_QUIT) {
                break;
            }
            last_ticks = cur_ticks;
            QueryPerformanceCounter(&cur_ticks);

            double time_taken = (cur_ticks.QuadPart - last_ticks.QuadPart) * 1000.f; 
            ctx.dt = time_taken / (double)ctx.pf_freq.QuadPart;
            app_desc->frame(&ctx);
            ctx.event_count = 0;
        }    
    }
end:
    app_desc->destroy(&ctx);
    return 0;
} 

void dd_destroy(dd_ctx* ctx)
{
    wglMakeCurrent(null, null);
}

double dd_time(dd_ctx* ctx)
{
    LARGE_INTEGER cur_ticks; QueryPerformanceCounter(&cur_ticks);
    double time_taken = (cur_ticks.QuadPart - ctx->start_time) * 1000.f; 
    return time_taken / (double)ctx->pf_freq.QuadPart;
}

HANDLE get_file_handle(const char* file_name, bool write) 
{
	HANDLE hFile;
	hFile = CreateFile(file_name,
		write ? GENERIC_WRITE : GENERIC_READ,
		write ? 0 : FILE_SHARE_READ,
		null,
		write ? CREATE_ALWAYS : OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL ,
		null);

    if (hFile == INVALID_HANDLE_VALUE) 
    {
        DWORD err = GetLastError();
        if (err == 2) {
            log_fatal("Datei %s wurde nicht gefunden", file_name);
            exit(-1);
        } 
        log_fatal("Fehler beim Öffnen der Datei, %lu", err);
        exit(-1);
    }

    return hFile;
}

char* dd_read_file(char* file_name, uint32_t* file_size)
{
    HANDLE hFile = get_file_handle(file_name, false);
    LARGE_INTEGER file_size_li;
    if (GetFileSizeEx(hFile, &file_size_li) == 0) {
        log_fatal("Fehler beim Abfragen der Größe der Datei, %lu", GetLastError());
        exit(-2);
    }
    uint32_t fs = file_size_li.QuadPart;

    char* buf = malloc(fs+1);
    DWORD read = 0;
    if (ReadFile(hFile, buf, fs, &read, null) == 0 || read == 0) {
        log_fatal("ERROR: Fehler beim Lesen der Datei, %lu", GetLastError());
        exit(-2);
    }
    CloseHandle(hFile);
    buf[fs] = '\0';
    if (file_size) {
        *file_size = fs;
    }
    return buf;
}