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
    typedef struct bs_vec2 bs_vec2;
    typedef struct bs_vec3 bs_vec3;
    typedef struct bs_vec4 bs_vec4;

    typedef struct bs_ivec2 bs_ivec2;
    typedef struct bs_ivec3 bs_ivec3;
    typedef struct bs_ivec4 bs_ivec4;

    typedef struct bs_uvec2 bs_uvec2;
    typedef struct bs_uvec3 bs_uvec3;
    typedef struct bs_uvec4 bs_uvec4;

    typedef struct bs_fRGBA bs_fRGBA;
    typedef struct bs_RGBA bs_RGBA;
    typedef struct bs_RGB bs_RGB;

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


struct bs_vec2 { float x, y; };
struct bs_vec3 { float x, y, z; };
struct bs_vec4 { float x, y, z, w; };

struct bs_ivec2 { int x, y; };
struct bs_ivec3 { int x, y, z; };
struct bs_ivec4 { int x, y, z, w; };
       
struct bs_uvec2 { bs_U32 x, y; };
struct bs_uvec3 { bs_U32 x, y, z; };
struct bs_uvec4 { bs_U32 x, y, z, w; };


struct bs_fRGBA { float r, g, b, a; };
struct bs_RGBA  { unsigned char r, g, b, a; };
struct bs_RGB   { unsigned char r, g, b; };

typedef BS_ALIGN_IF(16) bs_vec2 bs_mat2[2];
typedef BS_ALIGN_IF(16)	float bs_mat4[4][4];
typedef                 float bs_mat3[3][3];

typedef bs_vec4 	bs_quat;

/* --- ARRAY -> VECTORS --- */
// ARRAY -> VECTORS (INT)
#define BS_AIVEC2(arr) (bs_ivec2){ arr.x, arr.y }
#define BS_AIVEC3(arr) (bs_ivec3){ arr.x, arr.y, arr.z }
#define BS_AIVEC4(arr) (bs_ivec4){ arr.x, arr.y, arr.z, arr.w }
#define BS_AIV2(arr) BS_AIVEC2(arr)
#define BS_AIV3(arr) BS_AIVEC3(arr)
#define BS_AIV4(arr) BS_AIVEC4(arr)

// ARRAY -> VECTORS
#define BS_AVEC2(arr) (bs_vec2){ arr.x, arr.y }
#define BS_AVEC3(arr) (bs_vec3){ arr.x, arr.y, arr.z }
#define BS_AVEC4(arr) (bs_vec4){ arr.x, arr.y, arr.z, arr.w }
#define BS_AV2(arr) BS_AVEC2(arr)
#define BS_AV3(arr) BS_AVEC3(arr)
#define BS_AV4(arr) BS_AVEC4(arr)

/* --- COORDS -> VECTOR --- */
// COORDS -> VECTOR
#define BS_VEC2(x, y) (bs_vec2){ x, y }
#define BS_VEC3(x, y, z) (bs_vec3){ x, y, z }
#define BS_VEC4(x, y, z, w) (bs_vec4) { x, y, z, w }
#define BS_V2(x, y) BS_VEC2(x, y)
#define BS_V3(x, y, z) BS_VEC3(x, y, z)
#define BS_V4(x, y, z, w) BS_VEC4(x, y, z, w)

// COORDS -> VECTOR (INT)
#define BS_IVEC2(x, y) (bs_ivec2){ x, y }
#define BS_IVEC3(x, y, z) (bs_ivec3){ x, y, z }
#define BS_IVEC4(x, y, z, w) (bs_ivec4) { x, y, z, w }
#define BS_IV2(x, y) BS_IVEC2(x, y)
#define BS_IV3(x, y, z) BS_IVEC3(x, y, z)
#define BS_IV4(x, y, z, w) BS_IVEC4(x, y, z, w)

// "COORDS" -> COLOR
#define BS_RGBA(r, g, b, a) (bs_RGBA) { r, g, b, a }

// COORDS -> QUATERNIONS
#define BS_QUAT(x, y, z, w) (bs_quat){ x, y, z, w }

/* --- TEXTURES --- */
struct bs_Texture {
    bs_ivec3 frame;

    int w, h;
    float texw, texh;

    int type;
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
    BS_V4_,
    BS_V4I,

    BS_MAX_ATTRIB_COUNT
};

// TODO: Remove this shit
struct bs_Uniform {
    bool is_valid;
    int loc;
};

struct bs_Shader {
    bs_Uniform uniforms[BS_UNIFORM_TYPE_COUNT];
    uint8_t attrib_sizes[BS_MAX_ATTRIB_COUNT];
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

struct bs_MeshAnim {
    bs_mat4 *joints;
    int num_joints;
};

struct bs_Anim {
    bs_mat4 *matrices;
    bs_MeshAnim *mesh_anims;
    int num_mesh_anims;

    int joint_count;
    int frame_count;
    int frame_offset_shader;

    char *name;
};

struct bs_Material {
    bs_RGBA col;
    bs_Texture *tex;

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

    bs_Joint *joints;
    int joint_count;
};

struct bs_Model {
    char *name;

    bs_Mesh *meshes;
    bs_Texture *textures;

    int mesh_count;
    int prim_count;
    int vertex_count;
    int index_count;
};

/* --- VECTOR CONSTANTS --- */
#define BS_VEC2_0 (bs_vec2){ 0.0, 0.0 }
#define BS_VEC2_1 (bs_vec2){ 1.0, 1.0 }

#define BS_VEC3_0 (bs_vec3){ 0.0, 0.0, 0.0 }
#define BS_VEC3_1 (bs_vec3){ 1.0, 1.0, 1.0 }

#define BS_VEC4_0 (bs_vec4){ 0.0, 0.0, 0.0, 0.0 }
#define BS_VEC4_1 (bs_vec4){ 1.0, 1.0, 1.0, 1.0 }

#define BS_V2_0 BS_VEC2_0
#define BS_V2_1 BS_VEC2_1

#define BS_V3_0 BS_VEC3_0
#define BS_V3_1 BS_VEC3_1

#define BS_V4_0 BS_VEC4_0
#define BS_V4_1 BS_VEC4_1

#define BS_IVEC2_0 (bs_ivec4){ 0, 0 }
#define BS_IVEC2_1 (bs_ivec4){ 1, 1 }

#define BS_IVEC3_0 (bs_ivec4){ 0, 0, 0 }
#define BS_IVEC3_1 (bs_ivec4){ 1, 1, 1 }

#define BS_IVEC4_0 (bs_ivec4){ 0, 0, 0, 0 }
#define BS_IVEC4_1 (bs_ivec4){ 1, 1, 1, 1 }

#define BS_IV2_0 BS_IVEC2_0
#define BS_IV2_1 BS_IVEC2_1

#define BS_IV3_0 BS_IVEC3_0
#define BS_IV3_1 BS_IVEC3_1

#define BS_IV4_0 BS_IVEC4_0
#define BS_IV4_1 BS_IVEC4_1

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
