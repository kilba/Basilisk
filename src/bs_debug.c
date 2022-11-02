#include "bs_types.h"
#include <bs_core.h>
#include <bs_wnd.h>
#include <bs_textures.h>
#include <bs_shaders.h>

#include <stdio.h>
#include <stdarg.h>

#include <lodepng.h>
#include <cglm/cglm.h>
#include <glad/glad.h>

void bs_print(const int info_type, char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
}

void bs_printV2(bs_vec2 v) {
    printf("%f, %f\n", v.x, v.y);
}

void bs_printV3(bs_vec3 v) {
    printf("%f, %f, %f\n", v.x, v.y, v.z);
}

void bs_printV4(bs_vec4 v) {
    printf("%f, %f, %f, %f\n", v.x, v.y, v.z, v.w);
}

void bsdbg_tick() {
}

void bsdbg_init() {
}
