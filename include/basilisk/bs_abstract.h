#ifndef BS_ABSTRACT_H
#define BS_ABSTRACT_H

#include <bs_types.h>

void bs_drawTextureCam(bs_vec3 pos, bs_vec2 dim, bs_Texture *tex, bs_RGBA col, bs_Camera *cam);
void bs_drawFramebufPosCam(bs_Framebuf *framebuf, bs_vec2 pos, bs_vec2 dim, int attachment, bs_Camera *cam);
void bs_drawFramebufCam(bs_Framebuf *framebuf, int attachment, bs_Camera *cam);
void bs_drawRectCam(bs_vec3 pos, bs_vec2 dim, bs_RGBA col, bs_Camera *cam);
void bs_drawRectNDCCam(bs_vec3 pos, bs_vec2 dim, bs_RGBA col, bs_Camera *cam);
void bs_drawTriangleCam(bs_vec3 pos0, bs_vec3 pos1, bs_vec3 pos2, bs_RGBA col, bs_Camera *cam);
void bs_drawLineCam(bs_vec3 start, bs_vec3 end, bs_RGBA col, bs_Camera *cam);

void bs_drawTexture(bs_vec3 pos, bs_vec2 dim, bs_Texture *tex, bs_RGBA col);
void bs_drawFramebufPos(bs_Framebuf *framebuf, bs_vec2 pos, bs_vec2 dim, int attachment);
void bs_drawFramebuf(bs_Framebuf *framebuf, int attachment);
void bs_drawRect(bs_vec3 pos, bs_vec2 dim, bs_RGBA col);
void bs_drawTriangle(bs_vec3 pos0, bs_vec3 pos1, bs_vec3 pos2, bs_RGBA col);
void bs_drawRectNDC(bs_vec3 pos, bs_vec2 dim, bs_RGBA col);
void bs_drawLine(bs_vec3 start, bs_vec3 end, bs_RGBA col);

void bs_camAbstract(bs_Camera *cam);
void bs_initAbstract();

#endif /* BS_ABSTRACT_H */
