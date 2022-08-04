#ifndef BS_DEBUGGING_H
#define BS_DEBUGGING_H

void bs_print(const int info_type, char *format, ...);

// Debug Camera
bs_Camera *bs_getDebugCamera();
bool bs_debugCameraIsActivated();

void bs_debugUpdate();
void bs_debugStart();

// Comment out BS_DEBUG on release
// #define BS_DEBUG

#define BS_CLE 0
#define BS_INF 1
#define BS_WAR 2
#define BS_ERR 3

#endif /* BS_DEBUGGING_H */