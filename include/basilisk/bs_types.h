#ifndef BS_TYPES_H
#define BS_TYPES_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* CGLM Alignment */
#if defined(_MSC_VER)
#  if _MSC_VER < 1913 /*  Visual Studio 2017 version 15.6  */
#    define BS_ALL_UNALIGNED
#    define BS_ALIGN(X) /* no alignment */
#  else
#    define BS_ALIGN(X) __declspec(align(X))
#  endif
#else
#  define BS_ALIGN(X) __attribute((aligned(X)))
#endif

#ifndef BS_ALL_UNALIGNED
#  define BS_ALIGN_IF(X) BS_ALIGN(X)
#else
#  define BS_ALIGN_IF(X) /* no alignment */
#endif

#ifdef __AVX__
#  define BS_ALIGN_MAT BS_ALIGN(32)
#else
#  define BS_ALIGN_MAT BS_ALIGN(16)
#endif

#include <stdlib.h>
#include <stdio.h>

#ifndef __cplusplus
    typedef union bs_vec2 bs_vec2;
    typedef union bs_vec3 bs_vec3;
    typedef union bs_vec4 bs_vec4;
    typedef union bs_umat4 bs_umat4;

    typedef struct bs_ivec2 bs_ivec2;
    typedef struct bs_ivec3 bs_ivec3;
    typedef struct bs_ivec4 bs_ivec4;

    typedef struct bs_uvec2 bs_uvec2;
    typedef struct bs_uvec3 bs_uvec3;
    typedef struct bs_uvec4 bs_uvec4;

    typedef struct bs_aabb bs_aabb;

    typedef struct bs_fRGBA bs_fRGBA;
    typedef union  bs_RGBA bs_RGBA;
    typedef union  bs_RGB bs_RGB;

    typedef struct bs_AnimIdx bs_AnimIdx;
    typedef struct bs_Globals bs_Globals;
    typedef struct bs_Texture bs_Texture;
    typedef struct bs_Uniform bs_Uniform;
    typedef struct bs_Shader bs_Shader;
    typedef struct bs_ComputeShader bs_ComputeShader;
    typedef struct bs_UniformBuffer bs_UniformBuffer;
    typedef struct bs_Camera bs_Camera;
    typedef struct bs_Framebuf bs_Framebuf;
    typedef struct bs_Batch bs_Batch;
    typedef struct bs_MeshAnim bs_MeshAnim;
    typedef struct bs_Anim bs_Anim;
    typedef struct bs_Material bs_Material;
    typedef struct bs_Joint bs_Joint;
    typedef struct bs_Skin bs_Skin;
    typedef struct bs_Prim bs_Prim;
    typedef struct bs_Mesh bs_Mesh;
    typedef struct bs_Model bs_Model;
#endif 

typedef int64_t bs_I64, bs_long;
typedef int32_t bs_I32, bs_int;
typedef int16_t bs_I16, bs_short;
typedef int8_t  bs_I8 , bs_byte;

typedef uint64_t bs_U64, bs_ulong;
typedef uint32_t bs_U32, bs_uint;
typedef uint16_t bs_U16, bs_ushort;
typedef uint8_t  bs_U8 , bs_ubyte;


union bs_vec2 { 
    float a[2];
    struct { float x, y; };

#ifdef __cplusplus
    inline bs_vec2 operator+(bs_vec2 a) {
        return { x + a.x, y + a.y };
    }

    inline bs_vec2 operator-(bs_vec2 a) {
        return { x - a.x, y - a.y };
    }

    inline bs_vec2 operator*(bs_vec2 a) {
        return { x * a.x, y * a.y };
    }

    inline bs_vec2 operator/(bs_vec2 a) {
        return { x / a.x, y / a.y };
    }
#endif 
};

union bs_vec3 { 
    float a[3];
    struct { float x, y, z; };

#ifdef __cplusplus
    inline bs_vec3 operator+(bs_vec3 a) {
        return { x + a.x, y + a.y, z + a.z };
    }

    inline bs_vec3 operator-(bs_vec3 a) {
        return { x - a.x, y - a.y, z - a.z };
    }

    inline bs_vec3 operator*(bs_vec3 a) {
        return { x * a.x, y * a.y, z * a.z };
    }

    inline bs_vec3 operator/(bs_vec3 a) {
        return { x / a.x, y / a.y, z / a.z };
    }
#endif 
};

union bs_vec4 {
    float a[4];
    struct { float x, y, z, w; };

#ifdef __cplusplus
    inline bs_vec4 operator+(bs_vec4 a) {
        return { x + a.x, y + a.y, z + a.z, w + a.w };
    }

    inline bs_vec4 operator-(bs_vec4 a) {
        return { x - a.x, y - a.y, z - a.z, w - a.w };
    }

    inline bs_vec4 operator*(bs_vec4 a) {
        return { x * a.x, y * a.y, z * a.z, w * a.w };
    }

    inline bs_vec4 operator/(bs_vec4 a) {
        return { x / a.x, y / a.y, z / a.z, w / a.w };
    }
#endif 
};

struct bs_ivec2 { int x, y; };
struct bs_ivec3 { int x, y, z; };
struct bs_ivec4 { int x, y, z, w; };
       
struct bs_uvec2 { bs_U32 x, y; };
struct bs_uvec3 { bs_U32 x, y, z; };
struct bs_uvec4 { bs_U32 x, y, z, w; };


struct bs_fRGBA { float r, g, b, a; };
union  bs_RGBA  { struct { unsigned char r, g, b, a; }; bs_U32 hex; };
union  bs_RGB   { struct { unsigned char r, g, b;    }; bs_U32 hex : 24; };
union  bs_umat4  { float a[4][4]; bs_vec4 v[4]; };

typedef BS_ALIGN_IF(16) bs_vec2 bs_mat2[2];
typedef BS_ALIGN_IF(16)	bs_umat4 bs_mat4;
typedef                 float bs_mat3[3][3];

typedef bs_vec4 	bs_quat;

/* --- VECX --- */
#define BS_PTR(a) (float *)&a

#define BS_IVEC2_TO_VEC2(a) (bs_vec2){ { (float)a.x, (float)a.y } }
#define BS_IVEC3_TO_VEC3(a) (bs_vec3){ { (float)a.x, (float)a.y, (float)a.z } }
#define BS_IVEC4_TO_VEC4(a) (bs_vec4){ { (float)a.x, (float)a.y, (float)a.z, (float)a.w } }
#define BS_IV2_TO_V2(a) BS_IVEC2_TO_VEC2(a)
#define BS_IV3_TO_V3(a) BS_IVEC3_TO_VEC3(a)
#define BS_IV4_TO_V4(a) BS_IVEC4_TO_VEC4(a)

#define BS_VEC2_Z(a, b) (bs_vec3){ a.x, a.y, b }
#define BS_V2_Z(a, b) BS_VEC2_Z(a, b)

#define BS_XY(a) (bs_vec2){ a.x, a.y }
#define BS_XZ(a) (bs_vec2){ a.x, a.z }
#define BS_XW(a) (bs_vec2){ a.x, a.w }

#define BS_YX(a) (bs_vec2){ a.y, a.x }
#define BS_YZ(a) (bs_vec2){ a.y, a.z }
#define BS_YW(a) (bs_vec2){ a.y, a.w }

#define BS_ZX(a) (bs_vec2){ a.z, a.x }
#define BS_ZY(a) (bs_vec2){ a.z, a.y }
#define BS_ZW(a) (bs_vec2){ a.z, a.w }

#define BS_WX(a) (bs_vec2){ a.w, a.x }
#define BS_WY(a) (bs_vec2){ a.w, a.y }
#define BS_WZ(a) (bs_vec2){ a.w, a.z }

/* --- IVECX --- */
#define BS_IVEC2(x, y) (bs_ivec2){ x, y }
#define BS_IVEC3(x, y, z) (bs_ivec3){ x, y, z }
#define BS_IVEC4(x, y, z, w) (bs_ivec4) { x, y, z, w }
#define BS_IV2(x, y) BS_IVEC2(x, y)
#define BS_IV3(x, y, z) BS_IVEC3(x, y, z)
#define BS_IV4(x, y, z, w) BS_IVEC4(x, y, z, w)

#define BS_VEC2_TO_IVEC2(a) (bs_ivec2) { (int)a.x, (int)a.y }
#define BS_VEC3_TO_IVEC3(a) (bs_ivec3) { (int)a.x, (int)a.y, (int)a.z }
#define BS_VEC4_TO_IVEC4(a) (bs_ivec4) { (int)a.x, (int)a.y, (int)a.z, (int)a.w }
#define BS_V2_TO_IV2(a) BS_VEC2_TO_IVEC2(a)
#define BS_V3_TO_IV3(a) BS_VEC3_TO_IVEC3(a)
#define BS_V4_TO_IV4(a) BS_VEC4_TO_IVEC4(a)

#define BS_RGBA(r, g, b, a) (bs_RGBA) { r, g, b, a }

#define BS_AABB(a, b) (bs_aabb) { a, b }

struct bs_aabb {
    bs_vec3 min;
    bs_vec3 max;
};

/* --- TEXTURES --- */
struct bs_Texture {
    bs_ivec3 frame;
    int num_frames;

    int w, h;
    float texw, texh;

    int type;
    int attachment;
    unsigned int id;
    unsigned int unit;
    unsigned char *data;
};

/* --- SHADERS --- */
enum {
    UNIFORM_PROJ,
    UNIFORM_VIEW,

    BS_UNIFORM_TYPE_COUNT,
};

enum {
    BS_POS,
    BS_TEX,
    BS_COL,
    BS_NOR,
    BS_BID,
    BS_WEI,
    BS_VAR,
    BS_V4_,
    BS_V1_,

    BS_MAX_ATTRIB_COUNT
};

struct bs_AnimIdx {
    bs_U32 frame;
    bs_U32 num_frames;
};

struct bs_Globals {
    bs_ivec2 res;
    float elapsed;
    bs_vec3 cam_pos;
};

// TODO: Remove this shit
struct bs_Uniform {
    bool is_valid;
    int loc;
};

struct bs_Shader {
    bs_Uniform uniforms[BS_UNIFORM_TYPE_COUNT];
    uint8_t attrib_sizes[BS_MAX_ATTRIB_COUNT];
    int attrib_size_bytes;
    int attrib_count;
    int attribs;

    // OpenGL Variables
    unsigned int id;
    unsigned int vs_id;
    unsigned int fs_id;
    unsigned int gs_id;
};

struct bs_ComputeShader {
    unsigned int id;
    unsigned int cs_id;
    
    bs_Texture *tex;
};

struct bs_UniformBuffer {
    unsigned int id;
    unsigned int block_size;
};

struct bs_Camera {
    bs_mat4 view;
    bs_mat4 proj;
};

struct bs_Framebuf {
    bs_ivec2 dim;

    unsigned int FBO, RBO;
    int buffer_count;

    int clear;
    int culling;

    int buf_count;
    int buf_alloc;
    bs_Texture *bufs;

    bs_U8 depth_index;
};

// Contains all objects queued to render the next frame (unless using multiple batches)
struct bs_Batch {
    bs_Shader *shader;
    bs_Camera *camera;

    void *vertices;
    int *indices;

    int draw_mode;
    int vertex_draw_count;
    int index_draw_count;

    int attrib_count;
    int attrib_size_bytes;
    size_t attrib_offset;

    int allocated_index_count;
    int allocated_vertex_count;

    unsigned int VAO, VBO, EBO;
};

struct bs_Material {
    bs_RGBA col;
    int tex_idx;

    float metallic;
};

struct bs_Joint {
    // "mat" needs to be the first variable
    bs_mat4 mat;
    bs_mat4 local_inv;
    bs_mat4 bind_matrix;
    bs_mat4 bind_matrix_inv;

    bs_Joint *parent;
    int loc;
};

struct bs_Skin {
    bs_Joint *joints;
    int joint_count;

    char *name;
};

struct bs_Prim {
    bs_Material material;

    void *vertices;
    int vertex_count;
    int vertex_size;

    int offset_tex;
    int offset_nor;
    int offset_bid;
    int offset_wei;
    int offset_idx;

    int *indices;
    int index_count;
    int unique_count;
};

struct bs_Mesh {
    char *name; /* Can be freed */

    bs_vec3 pos;
    bs_vec4 rot;
    bs_vec3 sca;
    bs_mat4 mat;

    bs_Prim *prims;
    int prim_count;

    int vertex_count;
    int index_count;

    unsigned int id;
};

struct bs_Model {
    char *name;
    char **texture_names;

    bs_Skin *skins;
    bs_Mesh *meshes;

    int skin_count;
    int mesh_count;
    int prim_count;
    
    int anim_count;
    int anim_offset;

    int vertex_count;
    int index_count;
    
    int texture_count;
};

struct bs_MeshAnim {
    bs_mat4 *joints;
    int num_joints;
    int num_frames;
    int shader_offset;
};

struct bs_Anim {
    bs_Mesh *mesh;
    bs_mat4 *matrices;
    bs_MeshAnim *mesh_anims;
    int num_mesh_anims;

    int joint_count;
    int frame_count;
    int frame_offset_shader;

    char *name;
};

/* --- VECTOR CONSTANTS --- */
#define BS_VEC2_0 (bs_vec2){ { 0.0, 0.0 } }
#define BS_VEC2_1 (bs_vec2){ { 1.0, 1.0 } }

#define BS_VEC3_0 (bs_vec3){ { 0.0, 0.0, 0.0 } }
#define BS_VEC3_1 (bs_vec3){ { 1.0, 1.0, 1.0 } }

#define BS_VEC4_0 (bs_vec4){ { 0.0, 0.0, 0.0, 0.0 } }
#define BS_VEC4_1 (bs_vec4){ { 1.0, 1.0, 1.0, 1.0 } }

#define BS_V2_0 BS_VEC2_0
#define BS_V2_1 BS_VEC2_1

#define BS_V3_0 BS_VEC3_0
#define BS_V3_1 BS_VEC3_1

#define BS_V4_0 BS_VEC4_0
#define BS_V4_1 BS_VEC4_1

#define BS_IVEC2_0 (bs_ivec2){ 0, 0 }
#define BS_IVEC2_1 (bs_ivec2){ 1, 1 }

#define BS_IVEC3_0 (bs_ivec3){ 0, 0, 0 }
#define BS_IVEC3_1 (bs_ivec3){ 1, 1, 1 }

#define BS_IVEC4_0 (bs_ivec4){ 0, 0, 0, 0 }
#define BS_IVEC4_1 (bs_ivec4){ 1, 1, 1, 1 }

#define BS_IV2_0 BS_IVEC2_0
#define BS_IV2_1 BS_IVEC2_1

#define BS_IV3_0 BS_IVEC3_0
#define BS_IV3_1 BS_IVEC3_1

#define BS_IV4_0 BS_IVEC4_0
#define BS_IV4_1 BS_IVEC4_1

/* --- MATRIX CONSTANTS --- */
#define BS_MAT4_IDENTITY_INIT {{{1.0f, 0.0f, 0.0f, 0.0f}, \
                                {0.0f, 1.0f, 0.0f, 0.0f}, \
                                {0.0f, 0.0f, 1.0f, 0.0f}, \
                                {0.0f, 0.0f, 0.0f, 1.0f}}}

/* --- QUATERNION CONSTANTS --- */
#define BS_QUAT_IDENTITY (bs_quat) { { 0.0, 0.0, 0.0, 1.0 } }

/* --- COLOR CONSTANTS --- */
// bs_RGBA Constants
#define BS_BLANK (bs_RGBA){ 0, 0, 0, 0 }
#define BS_BLACK (bs_RGBA){ 0, 0, 0, 255 }
#define BS_WHITE (bs_RGBA){ 255, 255, 255, 255 }

#define BS_RED     (bs_RGBA){ 255, 0  , 0  , 255 }
#define BS_GREEN   (bs_RGBA){ 0  , 255, 0  , 255 }
#define BS_BLUE    (bs_RGBA){ 0  , 0  , 255, 255 }

#define BS_CYAN    (bs_RGBA){ 0  , 255, 255, 255 }
#define BS_YELLOW  (bs_RGBA){ 255, 255, 0  , 255 }
#define BS_MAGENTA (bs_RGBA){ 255, 0  , 255, 255 }

#endif /* BS_TYPES_H */
