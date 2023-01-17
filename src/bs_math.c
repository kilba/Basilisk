#include <bs_math.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

double bs_clamp(double d, double min, double max) {
  const double t = d < min ? min : d;
  return t > max ? max : t;
}

int bs_sign(float x) {
    return (x > 0) - (x < 0);
}

float bs_fsign(float x) {
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

bs_aabb bs_v3bounds(bs_vec3 *arr, int num_indices) {
    bs_aabb aabb = { BS_V3_0, BS_V3_0 };
    float far = -99999.0;
    float near = 99999.0;

    for(int i = 0; i < num_indices; i++) {
	float dot = bs_dot(BS_V3_1, arr[i]);
	if(dot > far) {
	    far = dot;
	    aabb.min = arr[i];
	}

	if(dot < near) {
	    near = dot;
	    aabb.max = arr[i];
	}
    }

    return aabb;
}

bs_vec2 bs_v2normalize(bs_vec2 v) {
    float w = sqrt(v.x * v.x + v.y * v.y);
    v.x /= w;
    v.y /= w;
    return v;
}

bs_vec3 bs_v3normalize(bs_vec3 v) {
    float w = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    v.x /= w;
    v.y /= w;
    v.z /= w;
    return v;
}

bs_vec3 bs_cross(bs_vec3 v0, bs_vec3 v1) {
    bs_vec3 out;
    out.x = v0.y * v1.z - v0.z * v1.y;
    out.y = v0.z * v1.x - v0.x * v1.z;
    out.z = v0.x * v1.y - v0.y * v1.x;
    return out;
}

float bs_dot(bs_vec3 v0, bs_vec3 v1) {
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

bs_vec3 bs_triangleNormal(bs_vec3 v0, bs_vec3 v1, bs_vec3 v2) {
    bs_vec3 a = BS_V3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    bs_vec3 b = BS_V3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);
    bs_vec3 c = bs_cross(a, b);
    c = bs_v3normalize(c);
    return c;
}

bs_vec3 bs_triangleCenter(bs_vec3 v0, bs_vec3 v1, bs_vec3 v2) {
    float x, y, z;
    x = v0.x + v1.x + v2.x;
    y = v0.y + v1.y + v2.y;
    z = v0.z + v1.z + v2.z;
    return BS_V3(x / 3.0, y / 3.0, z / 3.0);
}

bool bs_triangleIsCCW(bs_vec3 a, bs_vec3 b, bs_vec3 c, bs_vec3 normal) {
    return bs_cross(
	BS_V3(b.x - a.x, b.y - a.y, b.z - a.z), 
	BS_V3(c.x - a.x, c.y - a.y, c.z - a.z)
    ).z > 0.0;
}

float bs_signv3(bs_vec3 p1, bs_vec3 p2, bs_vec3 p3) {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bs_vec3 bs_v3mid(bs_vec3 a, bs_vec3 b) {
    return BS_V3((a.x + b.x) / 2.0, (a.y + b.y) / 2.0, (a.z + b.z) / 2.0);
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

bs_vec2 bs_v2rot(bs_vec2 pt, bs_vec2 origin, float angle) {
    angle = angle * BS_PI / 180.0;

    return BS_V2(
	cos(angle) * (pt.x - origin.x) - sin(angle) * (pt.y - origin.y) + origin.x,
	sin(angle) * (pt.x - origin.x) + cos(angle) * (pt.y - origin.y) + origin.y
    );
}

/* --- QUATERNIONS --- */
void bs_qToMat3(bs_vec4 q, bs_mat3 *out) {
    float qx2 = q.x + q.x;
    float qy2 = q.y + q.y;
    float qz2 = q.z + q.z;
    float qxqx2 = q.x * qx2;
    float qxqy2 = q.x * qy2;
    float qxqz2 = q.x * qz2;
    float qxqw2 = q.w * qx2;
    float qyqy2 = q.y * qy2;
    float qyqz2 = q.y * qz2;
    float qyqw2 = q.w * qy2;
    float qzqz2 = q.z * qz2;
    float qzqw2 = q.w * qz2;

    bs_mat3 cpy = {
	{ 1.0 - qyqy2 - qzqz2, qxqy2 + qzqw2, qxqz2 - qyqw2 },
	{ qxqy2 - qzqw2, 1.0 - qxqx2 - qzqz2, qyqz2 + qxqw2 },
	{ qxqz2 + qyqw2, qyqz2 - qxqw2, 1.0 - qxqx2 - qyqy2 }
    };

    memcpy(out, &cpy, sizeof(bs_mat3));
}

bs_quat bs_qMulq(bs_quat q, bs_quat rhs) {
    return BS_QUAT(
	    q.w * rhs.x + q.x * rhs.w + q.y * rhs.z - q.z * rhs.y,
	    q.w * rhs.y + q.y * rhs.w + q.z * rhs.x - q.x * rhs.z,
	    q.w * rhs.z + q.z * rhs.w + q.x * rhs.y - q.y * rhs.x,
	    q.w * rhs.w - q.x * rhs.x - q.y * rhs.y - q.z * rhs.z
	    );
}

bs_quat bs_qNormalize(bs_quat q) {
    float x = q.x;
    float y = q.y;
    float z = q.z;
    float w = q.w;

    float d = q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z;
    if(d == 0)
	w = 1.0;

    d = 1.0 / sqrt(d);
    if(d > (1.0e-8)) {
	x *= d;
	y *= d;
	z *= d;
	w *= d;
    }

    return BS_QUAT(x, y, z, w);
}

bs_quat bs_qIntegrate(bs_vec4 quat, bs_vec3 dv, float dt) {
    bs_quat q = { dv.x * dt, dv.y * dt, dv.z * dt, 0.0 };
    q = bs_qMulq(q, quat);

    quat.x += q.x * 0.5;
    quat.y += q.y * 0.5;
    quat.z += q.z * 0.5;
    quat.w += q.w * 0.5;

    return bs_qNormalize(quat);
}

/* --- BEZIER --- */
double bs_bezierScalar(double p0, double p1, double p2, double p3, double t) {
    double curve;

    curve  =     pow(1.0 - t, 3.0) * p0;
    curve += 3 * pow(1.0 - t, 2.0) * t * p1;
    curve += 3 * pow(1.0 - t, 1.0) * pow(t, 2.0) * p2;
    curve +=     pow(t, 3.0) * p3;

    return curve;
}

void bs_cubicBezierPts(bs_vec3 p0, bs_vec3 p1, bs_vec3 p2, bs_vec3 p3, bs_vec3 *arr, int num_elems) {
    double t = 0.0;
    double incr;
    int i = 0;

    incr = 1.0 / (double)num_elems;

    for(; i < num_elems; i++, t += incr) {
	float x = bs_bezierScalar(p0.x, p1.x, p2.x, p3.x, t);
	float y = bs_bezierScalar(p0.y, p1.y, p2.y, p3.y, t);
	float z = bs_bezierScalar(p0.z, p1.z, p2.z, p3.z, t);

	arr[i] = BS_V3(x, y, z);
    }
}

/* --- VECTOR ADDITION --- */
// VEC2
bs_vec2 bs_v2add(bs_vec2 a, bs_vec2 b) {
    return BS_V2(a.x + b.x, a.y + b.y);
}

// VEC3
bs_vec3 bs_v3add(bs_vec3 a, bs_vec3 b) {
    return BS_V3(a.x + b.x, a.y + b.y, a.z + b.z);
}

bs_vec3 bs_v3addv2(bs_vec3 a, bs_vec2 b) {
    return BS_V3(a.x + b.x, a.y + b.y, a.z);
}

// VEC4
bs_vec4 bs_v4add(bs_vec4 a, bs_vec4 b) {
    return BS_V4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

bs_vec4 bs_v4addv3(bs_vec4 a, bs_vec3 b) {
    return BS_V4(a.x + b.x, a.y + b.y, a.z + b.z, a.w);
}

/* --- VECTOR SUBTRACTION --- */
// VEC2
bs_vec2 bs_v2sub(bs_vec2 a, bs_vec2 b) {
    return BS_V2(a.x - b.x, a.y - b.y);
}

// VEC3
bs_vec3 bs_v3sub(bs_vec3 a, bs_vec3 b) {
    return BS_V3(a.x - b.x, a.y - b.y, a.z - b.z);
}

bs_vec3 bs_v3subv2(bs_vec3 a, bs_vec2 b) {
    return BS_V3(a.x - b.x, a.y - b.y, a.z);
}

// VEC4
bs_vec4 bs_v4sub(bs_vec4 a, bs_vec4 b) {
    return BS_V4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

bs_vec4 bs_v4subv3(bs_vec4 a, bs_vec3 b) {
    return BS_V4(a.x - b.x, a.y - b.y, a.z - b.z, a.w);
}

/* --- VECTOR MULTIPLICATION --- */
// VEC2
bs_vec2 bs_v2mul(bs_vec2 a, bs_vec2 b) {
    return BS_V2(a.x * b.x, a.y * b.y);
}

bs_vec2 bs_v2muls(bs_vec2 a, float s) {
    return BS_V2(a.x * s, a.y * s);
}

// VEC3
bs_vec3 bs_v3mul(bs_vec3 a, bs_vec3 b) {
    return BS_V3(a.x * b.x, a.y * b.y, a.z * b.z);
}

bs_vec3 bs_v3muls(bs_vec3 a, float s) {
    return BS_V3(a.x * s, a.y * s, a.z * s);
}

// VEC4
bs_vec4 bs_v4mul(bs_vec4 a, bs_vec4 b) {
    return BS_V4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

bs_vec4 bs_v4muls(bs_vec4 a, float s) {
    return BS_V4(a.x * s, a.y * s, a.z * s, a.w * s);
}

/* --- VECTOR DIVISION --- */
// VEC2
bs_vec2 bs_v2div(bs_vec2 a, bs_vec2 b) {
    return BS_V2(a.x / b.x, a.y / b.y);
}

bs_vec2 bs_v2divs(bs_vec2 a, float s) {
    return BS_V2(a.x / s, a.y / s);
}

// VEC3
bs_vec3 bs_v3div(bs_vec3 a, bs_vec3 b) {
    return BS_V3(a.x / b.x, a.y / b.y, a.z / b.z);
}

bs_vec3 bs_v3divs(bs_vec3 a, float s) {
    return BS_V3(a.x / s, a.y / s, a.z / s);
}

// VEC4
bs_vec4 bs_v4div(bs_vec4 a, bs_vec4 b) {
    return BS_V4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

bs_vec4 bs_v4divs(bs_vec4 a, float s) {
    return BS_V4(a.x / s, a.y / s, a.z / s, a.w / s);
}

/* --- VECTOR COMPARISON --- */
bool bs_v2cmp(bs_vec2 a, bs_vec2 b) {
    return (a.x == b.x) & (a.y == b.y);
}

bool bs_v3cmp(bs_vec3 a, bs_vec3 b) {
    return (a.x == b.x) & (a.y == b.y) & (a.z == b.z);
}

bool bs_v4cmp(bs_vec4 a, bs_vec4 b) {
    return (a.x == b.x) & (a.y == b.y) & (a.z == b.z) & (a.w == b.w);
}

/* --- RANDOM --- */
float bs_randRange(float min, float max) {
    float val = ((float)rand() / RAND_MAX) * max + min;
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
