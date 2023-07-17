#ifndef BS_MATH_H
#define BS_MATH_H

#include <bs_types.h>

#define BS_PI 3.142857
#define BS_SIN_45 0.70710678

#define BS_MIN(a,b) (((a)<(b))?(a):(b))
#define BS_MAX(a,b) (((a)>(b))?(a):(b))

float bs_degrees(float rad);
float bs_rad(float degrees);
double bs_clamp(double d, double min, double max);
int bs_sign(float x);
float bs_fsign(float x);
int bs_closestDivisible(int val, int div);
double bs_fMap(double input, double input_start, double input_end, double output_start, double output_end);
bs_quat bs_eul2quat(bs_vec3 eul);

bs_vec2 bs_v2normalize(bs_vec2 v);
bs_vec3 bs_v3normalize(bs_vec3 v);
bs_vec4 bs_v4normalize(bs_vec4 v);
bs_vec3 bs_cross(bs_vec3 v0, bs_vec3 v1);
float bs_v2dot(bs_vec2 v0, bs_vec2 v1);
float bs_v3dot(bs_vec3 v0, bs_vec3 v1);
float bs_v4dot(bs_vec4 v0, bs_vec4 v1);
bool bs_triangleIsCCW(bs_vec3 a, bs_vec3 b, bs_vec3 c, bs_vec3 normal);
float bs_signv3(bs_vec3 p1, bs_vec3 p2, bs_vec3 p3);
bool bs_ptInTriangle(bs_vec3 pt, bs_vec3 v1, bs_vec3 v2, bs_vec3 v3);
bs_vec3 bs_triangleNormal(bs_vec3 v0, bs_vec3 v1, bs_vec3 v2);
bs_vec3 bs_triangleCenter(bs_vec3 v0, bs_vec3 v1, bs_vec3 v2);
bs_vec2 bs_v2rot(bs_vec2 pt, bs_vec2 origin, float angle); 
bs_vec3 bs_v3mid(bs_vec3 a, bs_vec3 b);
bs_vec2 bs_v2mid(bs_vec2 a, bs_vec2 b);
bs_aabb bs_v3bounds(bs_vec3 *arr, int num_indices);
bs_vec3 bs_v3rotq(bs_vec3 v, bs_quat q);

/* --- QUATERNIONS --- */
void    bs_qToMat3(bs_vec4 q, bs_mat3 *out);
bs_quat bs_slerp(bs_quat q1, bs_quat q2, float t);
bs_quat bs_qMulq(bs_quat q, bs_quat rhs);
bs_quat bs_qNormalize(bs_quat q);
float   bs_qMagnitude(bs_quat q);
bs_quat bs_qIntegrate(bs_vec4 quat, bs_vec3 dv, float dt);

/* --- BEZIER --- */
double bs_sCubicBez(double p0, double p1, double p2, double p3, double t);
void bs_v2CubicBez(bs_vec2 p0, bs_vec2 p1, bs_vec2 p2, bs_vec2 p3, bs_vec2 *arr, int num_elems);
void bs_v2QuadBez(bs_vec2 p0, bs_vec2 p1, bs_vec2 p2, bs_vec2 *arr, int num_elems);
void bs_cubicBezierPts(bs_vec3 p0, bs_vec3 p1, bs_vec3 p2, bs_vec3 p3, bs_vec3 *arr, int num_elems);

/* --- MATRICES --- */
bs_vec4 bs_m4mulv4(bs_mat4 m, bs_vec4 v);
bs_mat4 bs_m4mul(bs_mat4 m1, bs_mat4 m2);
bs_mat4 bs_m4mulrot(bs_mat4 m1, bs_mat4 m2);
bs_mat4 bs_translate(bs_vec3 pos, bs_mat4 mat);
bs_mat4 bs_rotate(bs_quat rot, bs_mat4 mat);
bs_mat4 bs_scale(bs_vec3 sca, bs_mat4 mat);
bs_mat4 bs_transform(bs_vec3 pos, bs_quat rot, bs_vec3 sca);

/* --- VECTOR INITIALIZATION --- */
bs_ivec2 bs_iv2(int x, int y);
bs_ivec3 bs_iv3(int x, int y, int z);
bs_ivec4 bs_iv4(int x, int y, int z, int w);

bs_vec2 bs_v2(float x, float y);
bs_vec3 bs_v3(float x, float y, float z);
bs_vec4 bs_v4(float x, float y, float z, float w);
bs_quat bs_q(float x, float y, float z, float w);

bs_vec2 bs_v2s(float v);
bs_vec3 bs_v3s(float v);
bs_vec4 bs_v4s(float v);
/* --- VECTOR ADDITION --- */
bs_vec2 bs_v2add(bs_vec2 a, bs_vec2 b);
bs_vec3 bs_v3add(bs_vec3 a, bs_vec3 b);
bs_vec3 bs_v3addv2(bs_vec3 a, bs_vec2 b);
bs_vec4 bs_v4add(bs_vec4 a, bs_vec4 b);
bs_vec4 bs_v4addv3(bs_vec4 a, bs_vec3 b);

/* --- VECTOR SUBTRACTION --- */
bs_vec2 bs_v2sub(bs_vec2 a, bs_vec2 b);
bs_vec3 bs_v3sub(bs_vec3 a, bs_vec3 b);
bs_vec3 bs_v3subv2(bs_vec3 a, bs_vec2 b);
bs_vec4 bs_v4sub(bs_vec4 a, bs_vec4 b);
bs_vec4 bs_v4subv3(bs_vec4 a, bs_vec3 b);

/* --- VECTOR MULTIPLICATION --- */
bs_vec2 bs_v2mul(bs_vec2 a, bs_vec2 b);
bs_vec2 bs_v2muls(bs_vec2 a, float s);
bs_vec2 bs_v2muladds(bs_vec2 a, float s, bs_vec2 dest);
bs_vec3 bs_v3mul(bs_vec3 a, bs_vec3 b);
bs_vec3 bs_v3muls(bs_vec3 a, float s);
bs_vec3 bs_v3muladds(bs_vec3 a, float s, bs_vec3 dest);
bs_vec4 bs_v4mul(bs_vec4 a, bs_vec4 b);
bs_vec4 bs_v4muls(bs_vec4 a, float s);
bs_vec4 bs_v4muladds(bs_vec4 a, float s, bs_vec4 dest);

/* --- VECTOR DIVISION --- */
bs_vec2 bs_v2div(bs_vec2 a, bs_vec2 b);
bs_vec2 bs_v2divs(bs_vec2 a, float s);
bs_vec3 bs_v3div(bs_vec3 a, bs_vec3 b);
bs_vec3 bs_v3divs(bs_vec3 a, float s);
bs_vec4 bs_v4div(bs_vec4 a, bs_vec4 b);
bs_vec4 bs_v4divs(bs_vec4 a, float s);

/* --- VECTOR COMPARISON --- */
bool bs_v2cmp(bs_vec2 a, bs_vec2 b);
bool bs_v3cmp(bs_vec3 a, bs_vec3 b);
bool bs_v4cmp(bs_vec4 a, bs_vec4 b);

/* --- RANDOM --- */
int bs_randRangeI(int min, int max);
float bs_randRange(float min, float max);
bs_vec3 bs_randTrianglePt(bs_vec3 p0, bs_vec3 p1, bs_vec3 p2);

#endif /* BS_MATH_H */
