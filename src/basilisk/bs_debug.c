#include <bs_core.h>

#include <stdio.h>
#include <stdarg.h>

#include <cglm/cglm.h>

void bs_print(const int info_type, char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
}

void bs_debugUpdate() {

}

void bs_debugStart() {

}