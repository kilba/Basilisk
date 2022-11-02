#ifndef BS_DEBUGGING_H
#define BS_DEBUGGING_H

#include <bs_types.h>

void bs_print(const int info_type, char *format, ...);
void bs_printV2(bs_vec2 v);
void bs_printV3(bs_vec3 v);
void bs_printV4(bs_vec4 v);

void bsdbg_tick();
void bsdbg_init();

#define BS_CLE 0
#define BS_INF 1
#define BS_WAR 2
#define BS_ERR 3

#endif /* BS_DEBUGGING_H */
