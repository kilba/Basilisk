#include <windows.h>
#include <glad/glad.h>
#include <stdio.h>
#include <stdbool.h>

#include <bs_types.h>

// Zero-initialised array of every key's state
bool keys[256] = { 0 };
bs_fRGBA clear_color = { 0.0, 0.0, 0.0, 1.0 };

HWND hwnd;
HDC dc;
HGLRC rc;

struct bs_Globals {
    /* RAM / VRAM */
    float elapsed;

    /* RAM */
} bs_globals;

// Step 4: the Window Procedure
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

void bs_initWnd(int width, int height, char *title) {
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
        MessageBox(NULL, "Window Registration Failed11!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);

        printf("%d\n", GetLastError());
        return;
    }

    // Step 2: Creating the Window
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);

        printf("%d\n", GetLastError());
        return;
    }

    PIXELFORMATDESCRIPTOR pfd =
    {
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
    rc = wglCreateContext(dc);
    wglMakeCurrent(dc, rc);
    gladLoadGL();
    glViewport(0, 0, width, height);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
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

void bs_wndTick(void (*render)()) {
    MSG msg;

    double start;
    start = (double)GetTickCount64() / 1000.0;

    while(1) {
        glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT);

        render();
        SwapBuffers(dc);

        double curr;
        curr = (double)GetTickCount64() / 1000.0;
        // printf("%f\n", curr - start);

        // Check for inputs
        if (PeekMessage(&msg, NULL, 0, 255, PM_REMOVE) > 0) {
            switch(msg.message) {
                case WM_QUIT:
                    PostQuitMessage(0); return;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);

            continue;
        }

        PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
    }
    return;
}