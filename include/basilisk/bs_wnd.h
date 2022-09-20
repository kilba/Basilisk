#ifndef BS_WND_H
#define BS_WND_H

void bs_initWnd(int width, int height, char *title);
void bs_wndTick(void (*render)());
void bs_setBackgroundColor(bs_RGBA color);
void bs_tickSpeed(int fps);

bool bs_isKeyDown(int key_code);
bool bs_isKeyOnce(int key_code);
bool bs_isKeyDownOnce(int key_code);
bool bs_isKeyUpOnce(int key_code);

double bs_elapsedTime();
double bs_deltaTime();
bs_ivec2 bs_resolution();

#endif /* BS_WND_H */
