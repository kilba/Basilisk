#include <windows.h>
#include <glad/glad.h>
#include <stdio.h>
#include <stdbool.h>

#include <bs_types.h>
#include <bs_core.h>

// Zero-initialised array of every key's state
bool keys[256] = { 0 };
bs_fRGBA clear_color = { 0.0, 0.0, 0.0, 1.0 };

HWND hwnd;
HDC dc;
HGLRC rc;

int w, h;
double elapsed;
double delta_time;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void bs_initWnd(int width, int height, const char *title) {
    w = width;
    h = height;

    const char g_szClassName[] = "myWindowClass";
    WNDCLASSEX wc;

    HINSTANCE hInstance = GetModuleHandle(0);

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "myWindowClass";
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);

        return;
    }

    hwnd = CreateWindowEx(
        WS_EX_WINDOWEDGE,
        g_szClassName,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);

        return;
    }

    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    const int pixel_format = 11;

    dc = GetDC(hwnd);
    // int pf = ChoosePixelFormat(dc, &pfd);
    SetPixelFormat(dc, pixel_format, &pfd);
    HGLRC temprc = wglCreateContext(dc);

    wglMakeCurrent(dc, temprc);
    gladLoadGL();
/*
    int gl43Attribs[] = {
	    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
	    WGL_CONTEXT_MINOR_VERSION_ARB, 3,
	    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
	    0
    };

    rc = wglCreateContextAttribsARB(dc, 0, gl43Attribs);
    if(rc == NULL) {
	printf("%d\n", GetLastError());
	exit(1);
    }
    
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(temprc);
    wglMakeCurrent(dc, rc);

    */
    glViewport(0, 0, width, height);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // TODO: Get screen refresh rate
    SetTimer(hwnd, 999, 1000 / 120, NULL);
}

/* --- INPUTS --- */
bool bs_isKeyDown(int key_code) {
    SHORT tabKeyState = GetAsyncKeyState(key_code);
    keys[key_code] = tabKeyState & 0x8000;
    return keys[key_code];
}

bool bs_isKeyOnce(int key_code) {
    SHORT tabKeyState = GetAsyncKeyState(key_code);

    bool prior = keys[key_code];
    keys[key_code] = tabKeyState & 0x8000;

    return keys[key_code] != prior;
}

bool bs_isKeyDownOnce(int key_code) {
    bool key = bs_isKeyOnce(key_code);
    return key & keys[key_code];
}

bool bs_isKeyUpOnce(int key_code) {
    bool key = bs_isKeyOnce(key_code);
    return key & !keys[key_code];
}

/* --- WINDOW FUNCTIONS --- */
void bs_setBackgroundColor(bs_RGBA color) {
    clear_color.r = (float)color.r / 255.0;
    clear_color.g = (float)color.g / 255.0;
    clear_color.b = (float)color.b / 255.0;
    clear_color.a = (float)color.a / 255.0;
}

void bs_setBackgroundColorF(bs_fRGBA color) {
    clear_color = color;
}

bs_RGBA bs_getBackgroundColor() {
    return (bs_RGBA) {
	clear_color.r * 255.0,
	clear_color.g * 255.0,
	clear_color.b * 255.0,
	clear_color.a * 255.0
    };
}

bs_fRGBA bs_getBackgroundColorF() {
    return clear_color;
}

void bs_checkGLError() {
    GLenum err = glGetError();
    if(err != GL_NO_ERROR) {
        switch(err) {
            case GL_INVALID_ENUM                 : printf("INVALID_ENUM"); break;
            case GL_INVALID_VALUE                : printf("INVALID_VALUE"); break;
            case GL_INVALID_OPERATION            : printf("GL_INVALID_OPERATION"); break;
            case GL_STACK_OVERFLOW               : printf("GL_STACK_OVERFLOW"); break;
            case GL_STACK_UNDERFLOW              : printf("GL_STACK_UNDERFLOW"); break;
            case GL_OUT_OF_MEMORY                : printf("OUT_OF_MEMORY"); break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: printf("INVALID_FRAMEBUFFER_OPERATION"); break;
        }

        printf(" | 0x0%x\n", err);
    }
}

void bs_tickSpeed(int fps) {
    int fpms = 1000 / fps;
    SetTimer(hwnd, 999, fpms, NULL);
}

void bs_wndTick(void (*render)()) {
    MSG msg;

    double start, prev;
    start = (double)GetTickCount64() / 1000.0;

    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        switch(msg.message) {
            case WM_QUIT:
                PostQuitMessage(0); 
                return;
            case WM_TIMER:
                break;

            /* Skip scene update on non-timed events */
            default:
                goto pass;
        }

        prev = elapsed;

        elapsed = (double)GetTickCount64() / 1000.0;
        elapsed -= start;

        delta_time = elapsed - prev;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        bs_setGlobalVars();
        render();

        bs_checkGLError();
        SwapBuffers(dc);

        pass: continue;
    }
}

/* --- GET VARIABLES --- */
double bs_elapsedTime() {
    return elapsed;
}

double bs_deltaTime() {
    return delta_time;
}

bs_ivec2 bs_wndPosition() {
    RECT rect;
    GetWindowRect(hwnd, &rect);
    return BS_IVEC2(rect.left, rect.top);
}

bs_ivec2 bs_resolution() {
    return BS_IVEC2(w, h);
}

bs_ivec2 bs_wndResolution() {
    RECT rect;
    GetClientRect(hwnd, &rect);

    bs_ivec2 res;
    res.x = rect.right - rect.left;
    res.y = rect.bottom - rect.top;
    
    return res;
}

bs_ivec2 bs_cursorPos() {
    POINT p;
    GetCursorPos(&p);
    return BS_IVEC2(p.x, p.y);
}

bs_ivec2 bs_cursorPosWndInv() {
    bs_ivec2 cur_pos = bs_cursorPos();
    
    POINT p;
    p.x = cur_pos.x;
    p.y = cur_pos.y;

    // Map to window coords
    ScreenToClient(hwnd, &p);

    return BS_IVEC2(p.x, p.y);
}

bs_ivec2 bs_cursorPosWnd() {
    RECT rect;
    GetClientRect(hwnd, &rect);

    bs_ivec2 pos = bs_cursorPosWndInv();
    pos.y = rect.bottom - pos.y;

    return pos;
}
