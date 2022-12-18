#ifndef BS_MATH_H
#define BS_MATH_H

#include <bs_types.h>

double bs_clamp(double d, double min, double max);
int bs_sign(float x);
int bs_closestDivisible(int val, int div);
double bs_fMap(double input, double input_start, double input_end, double output_start, double output_end);
void bs_eul2quat(bs_quat q, bs_vec3 eul);

// void bs_normv3(bs_vec3 *v);
bs_vec3 bs_normalize(bs_vec3 v);
bs_vec3 bs_cross(bs_vec3 v0, bs_vec3 v1);
float bs_dot(bs_vec3 v0, bs_vec3 v1);
bool bs_triangleIsCCW(bs_vec3 a, bs_vec3 b, bs_vec3 c, bs_vec3 normal);
float bs_signv3(bs_vec3 p1, bs_vec3 p2, bs_vec3 p3);
bs_vec3 bs_v3mid(bs_vec3 a, bs_vec3 b);
bool bs_ptInTriangle(bs_vec3 pt, bs_vec3 v1, bs_vec3 v2, bs_vec3 v3);
bs_vec3 bs_triangleNormal(bs_vec3 v0, bs_vec3 v1, bs_vec3 v2);
bs_vec3 bs_triangleCenter(bs_vec3 v0, bs_vec3 v1, bs_vec3 v2);
bs_vec2 bs_v2rot(bs_vec2 pt, bs_vec2 origin, float angle); 

/* --- QUATERNIONS --- */
void bs_qToMat3(bs_vec4 q, bs_mat3 *out);
bs_quat bs_qMulq(bs_quat q, bs_quat rhs);
bs_quat bs_qNormalize(bs_quat q);
bs_quat bs_qIntegrate(bs_vec4 quat, bs_vec3 dv, float dt);

/* --- BEZIER --- */
double bs_bezierScalar(double p0, double p1, double p2, double p3, double t);
void bs_cubicBezierPts(bs_vec3 p0, bs_vec3 p1, bs_vec3 p2, bs_vec3 p3, bs_vec3 *arr, int num_elems);

/* --- VECTOR ADDITION --- */
bs_vec3 bs_v3add(bs_vec3 a, bs_vec3 b);
bs_vec3 bs_v3addv2(bs_vec3 a, bs_vec2 b);

/* --- VECTOR SUBTRACTION --- */
bs_vec3 bs_v3sub(bs_vec3 a, bs_vec3 b);
bs_vec3 bs_v3subv2(bs_vec3 a, bs_vec2 b);

/* --- VECTOR MULTIPLICATION --- */
bs_vec3 bs_v3mul(bs_vec3 a, bs_vec3 b);
bs_vec3 bs_v3muls(bs_vec3 a, float s);

/* --- VECTOR DIVISION --- */
bs_vec3 bs_v3div(bs_vec3 a, bs_vec3 b);
bs_vec3 bs_v3divs(bs_vec3 a, float s);

/* --- RANDOM --- */
float bs_randRange(float min, float max);
int bs_randRangeI(int min, int max);
bs_vec3 bs_randTrianglePt(bs_vec3 p0, bs_vec3 p1, bs_vec3 p2);

#endif /* BS_MATH_H */
