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

void bsdbg_tick() {
}

void bsdbg_init() {
}
