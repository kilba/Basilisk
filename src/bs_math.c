#include <bs_math.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int bs_sign(float x) {
    return (x > 0) - (x < 0);
}

int bs_closestDivisible(int val, int div) {
    int q = val / div;
     
    int n1 = div * q;
    int n2 = (val * div) > 0 ? (div * (q + 1)) : (div * (q - 1));
     
    if (abs(val - n1) < abs(val - n2))
        return n1;
     
    return n2;   
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

    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
}

void bs_normv3ret(bs_vec3 v, bs_vec3 out) {
    //memcpy(out, v, sizeof(bs_vec3));
    float w = sqrt(out.x * out.x + out.y * out.y + out.z * out.z);
    out.x /= w;
    out.y /= w;
    out.z /= w;
}

void bs_crossv3(bs_vec3 v0, bs_vec3 v1, bs_vec3 out) {
    out.x = v0.y * v1.z - v0.z * v1.y;
    out.y = v0.z * v1.x - v0.x * v1.z;
    out.z = v0.x * v1.y - v0.y * v1.x;
}

float bs_signv3(bs_vec3 p1, bs_vec3 p2, bs_vec3 p3) {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool bs_ptInTriangle(bs_vec3 pt, bs_vec3 v1, bs_vec3 v2, bs_vec3 v3) {
    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = bs_signv3(pt, v1, v2);
    d2 = bs_signv3(pt, v2, v3);
    d3 = bs_signv3(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

/* --- MATRICES --- */
void bs_mulMat4Vec4(bs_vec4 v, bs_mat4 m, bs_vec4 out) {
}

/* --- RANDOM --- */
float bs_randRange(float min, float max) {
    float val = ((float)rand()/RAND_MAX)*max + min;
    return val;
}

int bs_randRangeI(int min, int max) {
    int val = (rand() % (max - min + 1)) + max;
    return val;
}

bs_vec3 bs_randTrianglePt(bs_vec3 p0, bs_vec3 p1, bs_vec3 p2) {
    float a = bs_randRange(0, 1), b = bs_randRange(0, 1);
    if((a+b)>1.0) {
	a = 1.0 - a;
	b = 1.0 - b;
    }

    bs_vec3 v, v0;
    v.x = p1.x - p0.x;
    v.y = p1.y - p0.y;
    v.z = p1.z - p0.z;

    v.x *= a;
    v.y *= a;
    v.z *= a;

    v0.x = p2.x - p0.x;
    v0.y = p2.y - p0.y;
    v0.z = p2.z - p0.z;
    v0.x *= b;
    v0.y *= b;
    v0.z *= b;

    bs_vec3 val = p0;
    val.x += v.x;
    val.y += v.y;
    val.z += v.z;
    val.x += v0.x;
    val.y += v0.y;
    val.z += v0.z;

    return val;
}
