#include <bs_core.h>
#include <bs_wnd.h>
#include <bs_textures.h>
#include <bs_shaders.h>

#include <stdio.h>
#include <stdarg.h>

#include <lodepng.h>
#include <cglm/cglm.h>
#include <glad/glad.h>

void bs_rgbaprint(bs_RGBA rgba) {
    printf("%d, %d, %d, %d\n", rgba.r, rgba.g, rgba.b, rgba.r);
}

void bs_v2print(bs_vec2 v) {
    printf("%f, %f\n", v.x, v.y);
}

void bs_v3print(bs_vec3 v) {
    printf("%f, %f, %f\n", v.x, v.y, v.z);
}

void bs_v4print(bs_vec4 v) {
    printf("%f, %f, %f, %f\n", v.x, v.y, v.z, v.w);
}

void bs_iv2print(bs_ivec2 v) {
    printf("%d, %d\n", v.x, v.y);
}

void bs_iv3print(bs_ivec3 v) {
    printf("%d, %d, %d\n", v.x, v.y, v.z);
}

void bs_iv4print(bs_ivec4 v) {
    printf("%d, %d, %d, %d\n", v.x, v.y, v.z, v.w);
}

void bs_qprint(bs_quat q) {
    printf("%f, %f, %f, %f\n", q.x, q.y, q.z, q.w);
}

void bs_m3print(bs_mat3 m) {
    printf("%f, %f, %f\n"  , m[0][0], m[1][0], m[2][0]);
    printf("%f, %f, %f\n"  , m[0][1], m[1][1], m[2][1]);
    printf("%f, %f, %f\n\n", m[0][2], m[1][2], m[2][2]);
}

void bs_m4print(bs_mat4 m) {
    printf("%f, %f, %f, %f\n"  , m.a[0][0], m.a[1][0], m.a[2][0], m.a[3][0]);
    printf("%f, %f, %f, %f\n"  , m.a[0][1], m.a[1][1], m.a[2][1], m.a[3][1]);
    printf("%f, %f, %f, %f\n"  , m.a[0][2], m.a[1][2], m.a[2][2], m.a[3][2]);
    printf("%f, %f, %f, %f\n\n", m.a[0][3], m.a[1][3], m.a[2][3], m.a[3][3]);
}

void bs_print(const int info_type, char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
}
