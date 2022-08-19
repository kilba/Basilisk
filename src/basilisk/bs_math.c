#include <bs_math.h>
#include <math.h>

int bs_sign(float x) {
	return (x > 0) - (x < 0);
}

double bs_fMap(double input, double input_start, double input_end, double output_start, double output_end) {
	double slope = 1.0 * (output_end - output_start) / (input_end - input_start);
	return output_start + slope * (input - input_start);
}

void bs_eul2quat(bs_quat q, bs_vec3 eul) {
    double cy = cos(eul.x * 0.5);
    double sy = sin(eul.x * 0.5);
    double cp = cos(eul.y * 0.5);
    double sp = sin(eul.y * 0.5);
    double cr = cos(eul.z * 0.5);
    double sr = sin(eul.z * 0.5);

    q[3] = cr * cp * cy + sr * sp * sy;
    q[0] = sr * cp * cy - cr * sp * sy;
    q[1] = cr * sp * cy + sr * cp * sy;
    q[2] = cr * cp * sy - sr * sp * cy;
}

void bs_normv3(bs_vec3 *v) {
    float w = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
    v->x /= w;
    v->y /= w;
    v->z /= w;
}

bs_vec3 bs_normv3ret(bs_vec3 v) {
    float w = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    v.x /= w;
    v.y /= w;
    v.z /= w;

    return v;
}

bs_vec3 bs_crossv3(bs_vec3 v0, bs_vec3 v1) {
	bs_vec3 cross;
    cross.x = v0.y * v1.z - v0.z * v1.y;
    cross.y = v0.z * v1.x - v0.x * v1.z;
    cross.z = v0.x * v1.y - v0.y * v1.x;
    return cross;
}
