#include <bs_math.h>
#include <string.h>
#include <math.h>

int bs_sign(float x) {
	return (x > 0) - (x < 0);
}

double bs_fMap(double input, double input_start, double input_end, double output_start, double output_end) {
	double slope = 1.0 * (output_end - output_start) / (input_end - input_start);
	return output_start + slope * (input - input_start);
}

void bs_eul2quat(bs_quat q, bs_vec3 eul) {
    double cy = cos(eul[0] * 0.5);
    double sy = sin(eul[0] * 0.5);
    double cp = cos(eul[1] * 0.5);
    double sp = sin(eul[1] * 0.5);
    double cr = cos(eul[2] * 0.5);
    double sr = sin(eul[2] * 0.5);

    q[3] = cr * cp * cy + sr * sp * sy;
    q[0] = sr * cp * cy - cr * sp * sy;
    q[1] = cr * sp * cy + sr * cp * sy;
    q[2] = cr * cp * sy - sr * sp * cy;
}

void bs_normv3ret(bs_vec3 v, bs_vec3 out) {
	memcpy(out, v, sizeof(bs_vec3));
    float w = sqrt(out[0] * out[0] + out[1] * out[1] + out[2] * out[2]);
    out[0] /= w;
    out[1] /= w;
    out[2] /= w;
}

void bs_crossv3(bs_vec3 v0, bs_vec3 v1, bs_vec3 out) {
    out[0] = v0[1] * v1[2] - v0[2] * v1[1];
    out[1] = v0[2] * v1[0] - v0[0] * v1[2];
    out[2] = v0[0] * v1[1] - v0[1] * v1[0];
}
