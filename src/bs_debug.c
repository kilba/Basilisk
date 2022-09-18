#include <bs_core.h>

#include <stdio.h>
#include <stdarg.h>

#include <cglm/cglm.h>

bs_Camera cam;

void bs_print(const int info_type, char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
}

bool bs_debugCameraIsActivated() {
    return true;
}

bs_Camera *bs_getDebugCamera() {
    return &cam;
}

void bs_debugUpdate() {
}

void bs_debugStart() {
    printf("Debugging\n");
}