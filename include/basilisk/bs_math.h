#ifndef BS_MATH_H
#define BS_MATH_H

#include <bs_types.h>

int bs_sign(float x);
double bs_fMap(double input, double input_start, double input_end, double output_start, double output_end);
void bs_eul2quat(bs_quat q, bs_vec3 eul);

#endif /* BS_MATH_H */