#ifndef BS_WND_H
#define BS_WND_H

#include <bs_types.h>
#include <windows.h>

void bs_initWnd(int width, int height, const char *title);
void bs_wndTick(void (*render)());
void bs_setBackgroundColor(bs_RGBA color);
void bs_setBackgroundColorF(bs_fRGBA color);
bs_RGBA bs_getBackgroundColor();
bs_fRGBA bs_getBackgroundColorF();
void bs_tickSpeed(int fps);

HWND bs_hwnd();
void bs_wndProc(void (*func)(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam));

bool bs_isKeyDown(bs_U8 key_code);
bool bs_isKeyOnce(bs_U8 key_code);
bool bs_isKeyDownOnce(bs_U8 key_code);
bool bs_isKeyUpOnce(bs_U8 key_code);

double bs_elapsedTime();
double bs_deltaTime();
bs_ivec2 bs_resolution();
bs_ivec2 bs_wndResolution();
bs_ivec2 bs_cursorPos();
bs_ivec2 bs_cursorPosWnd();
bs_ivec2 bs_cursorPosWndInv();

#endif /* BS_WND_H */
