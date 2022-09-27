#ifndef BS_MATH_H
#define BS_MATH_H

#include <bs_types.h>

double bs_clamp(double d, double min, double max);
int bs_sign(float x);
int bs_closestDivisible(int val, int div);
double bs_fMap(double input, double input_start, double input_end, double output_start, double output_end);
void bs_eul2quat(bs_quat q, bs_vec3 eul);

// void bs_normv3(bs_vec3 *v);
void bs_normv3ret(bs_vec3 v, bs_vec3 out);
void bs_crossv3(bs_vec3 v0, bs_vec3 v1, bs_vec3 out);
float bs_signv3(bs_vec3 p1, bs_vec3 p2, bs_vec3 p3);
bool bs_ptInTriangle(bs_vec3 pt, bs_vec3 v1, bs_vec3 v2, bs_vec3 v3);

/* --- RANDOM --- */
float bs_randRange(float min, float max);
int bs_randRangeI(int min, int max);
bs_vec3 bs_randTrianglePt(bs_vec3 p0, bs_vec3 p1, bs_vec3 p2);

#endif /* BS_MATH_H */
