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

bs_quat bs_eul2quat(bs_vec3 eul) {
    bs_quat q;
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
    return q;
}

bs_aabb bs_v3bounds(bs_vec3 *arr, int num_indices) {
    bs_aabb aabb = { BS_V3_0, BS_V3_0 };
    float far = -99999.0;
    float near = 99999.0;

    for(int i = 0; i < num_indices; i++) {
	float dot = bs_v3dot(BS_V3_1, arr[i]);
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
    return bs_v2divs(v, sqrt(bs_v2dot(v, v)));
}

bs_vec3 bs_v3normalize(bs_vec3 v) {
    return bs_v3divs(v, sqrt(bs_v3dot(v, v)));
}

bs_vec4 bs_v4normalize(bs_vec4 v) {
    return bs_v4divs(v, sqrt(bs_v4dot(v, v)));
}

bs_vec3 bs_cross(bs_vec3 v0, bs_vec3 v1) {
    bs_vec3 out;
    out.x = v0.y * v1.z - v0.z * v1.y;
    out.y = v0.z * v1.x - v0.x * v1.z;
    out.z = v0.x * v1.y - v0.y * v1.x;
    return out;
}

float bs_v2dot(bs_vec2 v0, bs_vec2 v1) {
    return v0.x * v1.x + v0.y * v1.y;
}

float bs_v3dot(bs_vec3 v0, bs_vec3 v1) {
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

float bs_v4dot(bs_vec4 v0, bs_vec4 v1) {
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w;
}

bs_vec3 bs_triangleNormal(bs_vec3 v0, bs_vec3 v1, bs_vec3 v2) {
    bs_vec3 a = bs_v3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    bs_vec3 b = bs_v3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);
    bs_vec3 c = bs_cross(a, b);
    c = bs_v3normalize(c);
    return c;
}

bs_vec3 bs_triangleCenter(bs_vec3 v0, bs_vec3 v1, bs_vec3 v2) {
    float x, y, z;
    x = v0.x + v1.x + v2.x;
    y = v0.y + v1.y + v2.y;
    z = v0.z + v1.z + v2.z;
    return bs_v3(x / 3.0, y / 3.0, z / 3.0);
}

bool bs_triangleIsCCW(bs_vec3 a, bs_vec3 b, bs_vec3 c, bs_vec3 normal) {
    return bs_cross(
	bs_v3(b.x - a.x, b.y - a.y, b.z - a.z), 
	bs_v3(c.x - a.x, c.y - a.y, c.z - a.z)
    ).z > 0.0;
}

float bs_signv3(bs_vec3 p1, bs_vec3 p2, bs_vec3 p3) {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bs_vec3 bs_v3mid(bs_vec3 a, bs_vec3 b) {
    return bs_v3((a.x + b.x) / 2.0, (a.y + b.y) / 2.0, (a.z + b.z) / 2.0);
}

bs_vec2 bs_v2mid(bs_vec2 a, bs_vec2 b) {
    return bs_v2((a.x + b.x) / 2.0, (a.y + b.y) / 2.0);
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

    return bs_v2(
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
    return bs_q(
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

    return bs_q(x, y, z, w);
}

float bs_qMagnitude(bs_quat q) {
    return sqrtf(bs_v4dot(q, q));
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
double bs_sCubicBez(double p0, double p1, double p2, double p3, double t) {
    double curve;

    curve  =     pow(1.0 - t, 3.0) * p0;
    curve += 3 * pow(1.0 - t, 2.0) * t * p1;
    curve += 3 * pow(1.0 - t, 1.0) * pow(t, 2.0) * p2;
    curve +=     pow(t, 3.0) * p3;

    return curve;
}

void bs_v2CubicBez(bs_vec2 p0, bs_vec2 p1, bs_vec2 p2, bs_vec2 p3, bs_vec2 *arr, int num_elems) {
    double t = 0.0;
    double incr;
    int i = 0;

    incr = 1.0 / (double)num_elems;

    for(; i < num_elems; i++, t += incr) {
	float x = bs_sCubicBez(p0.x, p1.x, p2.x, p3.x, t);
	float y = bs_sCubicBez(p0.y, p1.y, p2.y, p3.y, t);

	arr[i] = bs_v2(x, y);
    }
}

void bs_v2QuadBez(bs_vec2 p0, bs_vec2 p1, bs_vec2 p2, bs_vec2 *arr, int num_elems) {
    double t = 0.0;
    double incr;

    incr = 1.0 / (double)num_elems;

    for(int i = 0; i < num_elems; i++, t += incr) {
	bs_vec2 v;
        v.x = (1 - t) * (1 - t) * p0.x + 2 * (1 - t) * t * p1.x + t * t * p2.x;
	v.y = (1 - t) * (1 - t) * p0.y + 2 * (1 - t) * t * p1.y + t * t * p2.y;

	arr[i] = v;
    }
}

void bs_cubicBezierPts(bs_vec3 p0, bs_vec3 p1, bs_vec3 p2, bs_vec3 p3, bs_vec3 *arr, int num_elems) {
    double t = 0.0;
    double incr;
    int i = 0;

    incr = 1.0 / (double)num_elems;

    for(; i < num_elems; i++, t += incr) {
	float x = bs_sCubicBez(p0.x, p1.x, p2.x, p3.x, t);
	float y = bs_sCubicBez(p0.y, p1.y, p2.y, p3.y, t);
	float z = bs_sCubicBez(p0.z, p1.z, p2.z, p3.z, t);

	arr[i] = bs_v3(x, y, z);
    }
}

/* --- MATRICES --- */
bs_mat4 bs_m4mulrot(bs_mat4 m1, bs_mat4 m2) {
    bs_mat4 dest;
  
    dest.a[0][0] = m1.a[0][0] * m2.a[0][0] + m1.a[1][0] * m2.a[0][1] + m1.a[2][0] * m2.a[0][2];
    dest.a[0][1] = m1.a[0][1] * m2.a[0][0] + m1.a[1][1] * m2.a[0][1] + m1.a[2][1] * m2.a[0][2];
    dest.a[0][2] = m1.a[0][2] * m2.a[0][0] + m1.a[1][2] * m2.a[0][1] + m1.a[2][2] * m2.a[0][2];
    dest.a[0][3] = m1.a[0][3] * m2.a[0][0] + m1.a[1][3] * m2.a[0][1] + m1.a[2][3] * m2.a[0][2];
    
    dest.a[1][0] = m1.a[0][0] * m2.a[1][0] + m1.a[1][0] * m2.a[1][1] + m1.a[2][0] * m2.a[1][2];    
    dest.a[1][1] = m1.a[0][1] * m2.a[1][0] + m1.a[1][1] * m2.a[1][1] + m1.a[2][1] * m2.a[1][2];   
    dest.a[1][2] = m1.a[0][2] * m2.a[1][0] + m1.a[1][2] * m2.a[1][1] + m1.a[2][2] * m2.a[1][2];  
    dest.a[1][3] = m1.a[0][3] * m2.a[1][0] + m1.a[1][3] * m2.a[1][1] + m1.a[2][3] * m2.a[1][2]; 
    
    dest.a[2][0] = m1.a[0][0] * m2.a[2][0] + m1.a[1][0] * m2.a[2][1] + m1.a[2][0] * m2.a[2][2];
    dest.a[2][1] = m1.a[0][1] * m2.a[2][0] + m1.a[1][1] * m2.a[2][1] + m1.a[2][1] * m2.a[2][2];
    dest.a[2][2] = m1.a[0][2] * m2.a[2][0] + m1.a[1][2] * m2.a[2][1] + m1.a[2][2] * m2.a[2][2];
    dest.a[2][3] = m1.a[0][3] * m2.a[2][0] + m1.a[1][3] * m2.a[2][1] + m1.a[2][3] * m2.a[2][2];

    dest.v[3] = bs_v4(m1.a[3][0], m1.a[3][1], m1.a[3][2], m1.a[3][3]);
    return dest;
}

bs_mat4 bs_translate(bs_vec3 pos, bs_mat4 mat) {
    mat.v[3] = bs_v4muladds(mat.v[0], pos.x, mat.v[3]); 
    mat.v[3] = bs_v4muladds(mat.v[1], pos.y, mat.v[3]); 
    mat.v[3] = bs_v4muladds(mat.v[2], pos.z, mat.v[3]); 
    return mat;
}

bs_mat4 bs_rotate(bs_quat rot, bs_mat4 mat) {
    float norm = bs_qMagnitude(rot);
    float s    = norm > 0.0f ? 2.0f / norm : 0.0f;

    float xx, yy, zz,   xy, yz, xz,   wx, wy, wz;
    xx = s * rot.x * rot.x;   xy = s * rot.x * rot.y;   wx = s * rot.w * rot.x;
    yy = s * rot.y * rot.y;   yz = s * rot.y * rot.z;   wy = s * rot.w * rot.y;
    zz = s * rot.z * rot.z;   xz = s * rot.x * rot.z;   wz = s * rot.w * rot.z;

    bs_mat4 rotm4 = {{
	{ 1.0 - yy - zz, xy + wz      , xz - wy      , 0.0 },
	{ xy - wz      , 1.0 - xx - zz, yz + wx      , 0.0 },
	{ xz + wy      , yz - wx      , 1.0 - xx - yy, 0.0 },
        { 0.0          , 0.0          , 0.0          , 1.0 }
    }};

    return bs_m4mulrot(mat, rotm4);
}

bs_mat4 bs_scale(bs_vec3 sca, bs_mat4 mat) {
  mat.v[0] = bs_v4muls(mat.v[0], sca.x);
  mat.v[1] = bs_v4muls(mat.v[1], sca.x);
  mat.v[2] = bs_v4muls(mat.v[2], sca.x);
  return mat;
}

bs_mat4 bs_transform(bs_vec3 pos, bs_quat rot, bs_vec3 sca) {
    bs_mat4 m = BS_MAT4_IDENTITY_INIT;
    m = bs_translate(pos, m);
    m = bs_rotate(rot, m);
    m = bs_scale(sca, m);

    return m;
}

/* --- VECTOR INITIALIZATION --- */
bs_vec2 bs_v2(float x, float y) {
    return (bs_vec2) { x, y };
}

bs_vec3 bs_v3(float x, float y, float z) {
    return (bs_vec3) { x, y, z };
}

bs_vec4 bs_v4(float x, float y, float z, float w) {
    return (bs_vec4) { x, y, z, w };
}

bs_quat bs_q(float x, float y, float z, float w) {
    return (bs_quat) { x, y, z, w };
}

/* --- VECTOR ADDITION --- */
// VEC2
bs_vec2 bs_v2add(bs_vec2 a, bs_vec2 b) {
    return bs_v2(a.x + b.x, a.y + b.y);
}

// VEC3
bs_vec3 bs_v3add(bs_vec3 a, bs_vec3 b) {
    return bs_v3(a.x + b.x, a.y + b.y, a.z + b.z);
}

bs_vec3 bs_v3addv2(bs_vec3 a, bs_vec2 b) {
    return bs_v3(a.x + b.x, a.y + b.y, a.z);
}

// VEC4
bs_vec4 bs_v4add(bs_vec4 a, bs_vec4 b) {
    return bs_v4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

bs_vec4 bs_v4addv3(bs_vec4 a, bs_vec3 b) {
    return bs_v4(a.x + b.x, a.y + b.y, a.z + b.z, a.w);
}

/* --- VECTOR SUBTRACTION --- */
// VEC2
bs_vec2 bs_v2sub(bs_vec2 a, bs_vec2 b) {
    return bs_v2(a.x - b.x, a.y - b.y);
}

// VEC3
bs_vec3 bs_v3sub(bs_vec3 a, bs_vec3 b) {
    return bs_v3(a.x - b.x, a.y - b.y, a.z - b.z);
}

bs_vec3 bs_v3subv2(bs_vec3 a, bs_vec2 b) {
    return bs_v3(a.x - b.x, a.y - b.y, a.z);
}

// VEC4
bs_vec4 bs_v4sub(bs_vec4 a, bs_vec4 b) {
    return bs_v4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

bs_vec4 bs_v4subv3(bs_vec4 a, bs_vec3 b) {
    return bs_v4(a.x - b.x, a.y - b.y, a.z - b.z, a.w);
}

/* --- VECTOR MULTIPLICATION --- */
// VEC2
bs_vec2 bs_v2mul(bs_vec2 a, bs_vec2 b) {
    return bs_v2(a.x * b.x, a.y * b.y);
}

bs_vec2 bs_v2muls(bs_vec2 a, float s) {
    return bs_v2(a.x * s, a.y * s);
}

bs_vec2 bs_v2muladds(bs_vec2 a, float s, bs_vec2 dest) {
    return bs_v2add(dest, bs_v2muls(a, s));
}

// VEC3
bs_vec3 bs_v3mul(bs_vec3 a, bs_vec3 b) {
    return bs_v3(a.x * b.x, a.y * b.y, a.z * b.z);
}

bs_vec3 bs_v3muls(bs_vec3 a, float s) {
    return bs_v3(a.x * s, a.y * s, a.z * s);
}

bs_vec3 bs_v3muladds(bs_vec3 a, float s, bs_vec3 dest) {
    return bs_v3add(dest, bs_v3muls(a, s));
}

// VEC4
bs_vec4 bs_v4mul(bs_vec4 a, bs_vec4 b) {
    return bs_v4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

bs_vec4 bs_v4muls(bs_vec4 a, float s) {
    return bs_v4(a.x * s, a.y * s, a.z * s, a.w * s);
}

bs_vec4 bs_v4muladds(bs_vec4 a, float s, bs_vec4 dest) {
    return bs_v4add(dest, bs_v4muls(a, s));
}

/* --- VECTOR DIVISION --- */
// VEC2
bs_vec2 bs_v2div(bs_vec2 a, bs_vec2 b) {
    return bs_v2(a.x / b.x, a.y / b.y);
}

bs_vec2 bs_v2divs(bs_vec2 a, float s) {
    return bs_v2(a.x / s, a.y / s);
}

// VEC3
bs_vec3 bs_v3div(bs_vec3 a, bs_vec3 b) {
    return bs_v3(a.x / b.x, a.y / b.y, a.z / b.z);
}

bs_vec3 bs_v3divs(bs_vec3 a, float s) {
    return bs_v3(a.x / s, a.y / s, a.z / s);
}

// VEC4
bs_vec4 bs_v4div(bs_vec4 a, bs_vec4 b) {
    return bs_v4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

bs_vec4 bs_v4divs(bs_vec4 a, float s) {
    return bs_v4(a.x / s, a.y / s, a.z / s, a.w / s);
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
