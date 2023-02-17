#ifndef BS_DEBUGGING_H
#define BS_DEBUGGING_H

#include <bs_types.h>

void bs_rgbaprint(bs_RGBA rgba);
void bs_v2print(bs_vec2 v);
void bs_v3print(bs_vec3 v);
void bs_v4print(bs_vec4 v);
void bs_iv2print(bs_ivec2 v);
void bs_iv3print(bs_ivec3 v);
void bs_iv4print(bs_ivec4 v);
void bs_qprint(bs_quat q);
void bs_m3print(bs_mat3 m);
void bs_m4print(bs_mat4 m);
void bs_print(const int info_type, char *format, ...);

#define BS_CLE 0
#define BS_INF 1
#define BS_WAR 2
#define BS_ERR 3

#endif /* BS_DEBUGGING_H */
