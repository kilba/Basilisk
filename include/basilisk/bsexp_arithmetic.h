/* ALL OF THIS IS EXPERIMENTAL! */

#include <bs_types.h>

/* --- VECTOR ARITHMETIC ---*/
/* ADDITION */
inline bs_vec2 BS_V2ADDV2(float *v2_0, float *v2_1) {
    return (bs_vec2){ v2_0[0] + v2_1[0], v2_0[1] + v2_1[1] };
}

inline bs_vec3 BS_V2ADDV3(float *v3, float *v2) {
    return (bs_vec3){ v2[0] + v3[0], v2[1] + v3[1], v3[2] };
}

inline bs_vec4 BS_V2ADDV4(float *v4, float *v3) {
    return (bs_vec4){ v4[0] + v3[0], v4[1] + v3[1], v4[2] + v3[2], v4[3] };
}

inline bs_vec3 BS_V3ADDV3(float *v3_0, float *v3_1) { 
    return (bs_vec3){ v3_0[0] + v3_1[0], v3_0[1] + v3_1[1], v3_0[2] + v3_1[2] };
}

inline bs_vec4 BS_V3ADDV4(float *v4, float *v3) {
    return (bs_vec4){ v4[0] + v3[0], v4[1] + v3[1], v4[2] + v3[2], v4[3] };
}

inline bs_vec4 BS_V4ADDV4(float *v4_0, float *v4_1) {
    return (bs_vec4){ v4_0[0] + v4_1[0], v4_0[1] + v4_1[1], v4_0[2] + v4_1[2], v4_0[3] + v4_1[3] };
}

/* SUBTRACTION */
inline bs_vec2 BS_V2SUBV2(float *v2_0, float *v2_1) {
    return (bs_vec2){ v2_0[0] - v2_1[0], v2_0[1] - v2_1[1] };
}

inline bs_vec3 BS_V2SUBV3(float *v3, float *v2) {
    return (bs_vec3){ v2[0] - v3[0], v2[1] - v3[1], v3[2] };
}

inline bs_vec4 BS_V2SUBV4(float *v4, float *v3) {
    return (bs_vec4){ v4[0] - v3[0], v4[1] - v3[1], v4[2] - v3[2], v4[3] };
}

inline bs_vec3 BS_V3SUBV3(float *v3_0, float *v3_1) { 
    return (bs_vec3){ v3_0[0] - v3_1[0], v3_0[1] - v3_1[1], v3_0[2] - v3_1[2] };
}

inline bs_vec4 BS_V3SUBV4(float *v4, float *v3) {
    return (bs_vec4){ v4[0] - v3[0], v4[1] - v3[1], v4[2] - v3[2], v4[3] };
}

inline bs_vec4 BS_V4SUBV4(float *v4_0, float *v4_1) {
    return (bs_vec4){ v4_0[0] - v4_1[0], v4_0[1] - v4_1[1], v4_0[2] - v4_1[2], v4_0[3] - v4_1[3] };
}

/* MULTIPLICATION */
inline bs_vec2 BS_V2MULV2(float *v2_0, float *v2_1) {
    return (bs_vec2){ v2_0[0] * v2_1[0], v2_0[1] * v2_1[1] };
}

inline bs_vec3 BS_V2MULV3(float *v3, float *v2) {
    return (bs_vec3){ v2[0] * v3[0], v2[1] * v3[1], v3[2] };
}

inline bs_vec4 BS_V2MULV4(float *v4, float *v3) {
    return (bs_vec4){ v4[0] * v3[0], v4[1] * v3[1], v4[2] * v3[2], v4[3] };
}

inline bs_vec3 BS_V3MULV3(float *v3_0, float *v3_1) { 
    return (bs_vec3){ v3_0[0] * v3_1[0], v3_0[1] * v3_1[1], v3_0[2] * v3_1[2] };
}

inline bs_vec4 BS_V3MULV4(float *v4, float *v3) {
    return (bs_vec4){ v4[0] * v3[0], v4[1] * v3[1], v4[2] * v3[2], v4[3] };
}

inline bs_vec4 BS_V4MULV4(float *v4_0, float *v4_1) {
    return (bs_vec4){ v4_0[0] * v4_1[0], v4_0[1] * v4_1[1], v4_0[2] * v4_1[2], v4_0[3] * v4_1[3] };
}

/* DIVISION */
inline bs_vec2 BS_V2DIVV2(float *v2_0, float *v2_1) {
    return (bs_vec2){ v2_0[0] / v2_1[0], v2_0[1] / v2_1[1] };
}

inline bs_vec3 BS_V2DIVV3(float *v3, float *v2) {
    return (bs_vec3){ v2[0] / v3[0], v2[1] / v3[1], v3[2] };
}

inline bs_vec4 BS_V2DIVV4(float *v4, float *v3) {
    return (bs_vec4){ v4[0] / v3[0], v4[1] / v3[1], v4[2] / v3[2], v4[3] };
}

inline bs_vec3 BS_V3DIVV3(float *v3_0, float *v3_1) { 
    return (bs_vec3){ v3_0[0] / v3_1[0], v3_0[1] / v3_1[1], v3_0[2] / v3_1[2] };
}

inline bs_vec4 BS_V3DIVV4(float *v4, float *v3) {
    return (bs_vec4){ v4[0] / v3[0], v4[1] / v3[1], v4[2] / v3[2], v4[3] };
}

inline bs_vec4 BS_V4DIVV4(float *v4_0, float *v4_1) {
    return (bs_vec4){ v4_0[0] / v4_1[0], v4_0[1] / v4_1[1], v4_0[2] / v4_1[2], v4_0[3] / v4_1[3] };
}

#define BS_ADD(v0, v1) _Generic((v0),			\
    bs_vec2: _Generic((v1),				\
	bs_vec2: BS_V2ADDV2((float*)&v0, (float*)&v1),	\
	bs_vec3: BS_V2ADDV3((float*)&v0, (float*)&v1),	\
	bs_vec4: BS_V2ADDV4((float*)&v0, (float*)&v1)	\
	),						\
    bs_vec3: _Generic((v1),				\
	bs_vec2: BS_V2ADDV3((float*)&v1, (float*)&v0),	\
	bs_vec3: BS_V3ADDV3((float*)&v1, (float*)&v0),	\
	bs_vec4: BS_V3ADDV4((float*)&v1, (float*)&v0)	\
	),						\
    bs_vec4: _Generic((v1),				\
	bs_vec2: BS_V2ADDV4((float*)&v0, (float*)&v1),	\
	bs_vec3: BS_V3ADDV4((float*)&v0, (float*)&v1),	\
	bs_vec4: BS_V4ADDV4((float*)&v0, (float*)&v1)	\
	)						\
    )

#define BS_SUB(v0, v1) _Generic((v0),			\
    bs_vec2: _Generic((v1),				\
	bs_vec2: BS_V2SUBV2((float*)&v0, (float*)&v1),	\
	bs_vec3: BS_V2SUBV3((float*)&v0, (float*)&v1),	\
	bs_vec4: BS_V2SUBV4((float*)&v0, (float*)&v1)	\
	),						\
    bs_vec3: _Generic((v1),				\
	bs_vec2: BS_V2SUBV3((float*)&v1, (float*)&v0),	\
	bs_vec3: BS_V3SUBV3((float*)&v1, (float*)&v0),	\
	bs_vec4: BS_V3SUBV4((float*)&v1, (float*)&v0)	\
	),						\
    bs_vec4: _Generic((v1),				\
	bs_vec2: BS_V2SUBV4((float*)&v0, (float*)&v1),	\
	bs_vec3: BS_V3SUBV4((float*)&v0, (float*)&v1),	\
	bs_vec4: BS_V4SUBV4((float*)&v0, (float*)&v1)	\
	)						\
    )

#define BS_MUL(v0, v1) _Generic((v0),			\
    bs_vec2: _Generic((v1),				\
	bs_vec2: BS_V2MULV2((float*)&v0, (float*)&v1),	\
	bs_vec3: BS_V2MULV3((float*)&v0, (float*)&v1),	\
	bs_vec4: BS_V2MULV4((float*)&v0, (float*)&v1)	\
	),						\
    bs_vec3: _Generic((v1),				\
	bs_vec2: BS_V2MULV3((float*)&v1, (float*)&v0),	\
	bs_vec3: BS_V3MULV3((float*)&v1, (float*)&v0),	\
	bs_vec4: BS_V3MULV4((float*)&v1, (float*)&v0)	\
	),						\
    bs_vec4: _Generic((v1),				\
	bs_vec2: BS_V2MULV4((float*)&v0, (float*)&v1),	\
	bs_vec3: BS_V3MULV4((float*)&v0, (float*)&v1),	\
	bs_vec4: BS_V4MULV4((float*)&v0, (float*)&v1)	\
	)						\
    )

#define BS_DIV(v0, v1) _Generic((v0),			\
    bs_vec2: _Generic((v1),				\
	bs_vec2: BS_V2DIVV2((float*)&v0, (float*)&v1),	\
	bs_vec3: BS_V2DIVV3((float*)&v0, (float*)&v1),	\
	bs_vec4: BS_V2DIVV4((float*)&v0, (float*)&v1)	\
	),						\
    bs_vec3: _Generic((v1),				\
	bs_vec2: BS_V2DIVV3((float*)&v1, (float*)&v0),	\
	bs_vec3: BS_V3DIVV3((float*)&v1, (float*)&v0),	\
	bs_vec4: BS_V3DIVV4((float*)&v1, (float*)&v0)	\
	),						\
    bs_vec4: _Generic((v1),				\
	bs_vec2: BS_V2DIVV4((float*)&v0, (float*)&v1),	\
	bs_vec3: BS_V3DIVV4((float*)&v0, (float*)&v1),	\
	bs_vec4: BS_V4DIVV4((float*)&v0, (float*)&v1)	\
	)						\
    )

