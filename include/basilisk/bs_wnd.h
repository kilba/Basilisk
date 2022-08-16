#ifndef BS_WND_H
#define BS_WND_H

void bs_initWnd(int width, int height, char *title);
void bs_wndTick(void (*render)());
void bs_setBackgroundColor(bs_RGBA color);

bool bs_isKeyDown(int key_code);
bool bs_isKeyOnce(int key_code);
bool bs_isKeyDownOnce(int key_code);
bool bs_isKeyUpOnce(int key_code);

#endif /* BS_WND_H */