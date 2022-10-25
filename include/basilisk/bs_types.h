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

typedef struct { float x, y; } bs_vec2;
typedef struct { float x, y, z; } bs_vec3;
typedef struct { float x, y, z, w; } bs_vec4;

typedef struct { int x, y; } bs_ivec2;
typedef struct { int x, y, z; } bs_ivec3;
typedef struct { int x, y, z, w; } bs_ivec4;

typedef struct { float r, g, b, a; } bs_fRGBA;
typedef struct { unsigned char r, g, b, a; } bs_RGBA;
typedef struct { unsigned char r, g, b; } bs_RGB;

/* --- FLOAT TYPES --- */

typedef BS_ALIGN_IF(16) bs_vec2 bs_mat2[2];
typedef BS_ALIGN_IF(16)	float bs_mat4[4][4];

typedef bs_vec4 	bs_quat;

/* --- ARRAY -> VECTORS --- */
// ARRAY -> VECTORS (INT)
#define BS_IVEC2(arr) (bs_ivec2){ arr.x, arr.y }
#define BS_IVEC3(arr) (bs_ivec3){ arr.x, arr.y, arr.z }
#define BS_IVEC4(arr) (bs_ivec4){ arr.x, arr.y, arr.z, arr.w }
#define BS_IV2(arr) BS_IVEC2(arr)
#define BS_IV3(arr) BS_IVEC3(arr)
#define BS_IV4(arr) BS_IVEC4(arr)

// ARRAY -> VECTORS
#define BS_VEC2(arr) (bs_vec2){ arr.x, arr.y }
#define BS_VEC3(arr) (bs_vec3){ arr.x, arr.y, arr.z }
#define BS_VEC4(arr) (bs_vec4){ arr.x, arr.y, arr.z, arr.w }
#define BS_V2(arr) BS_VEC2(arr)
#define BS_V3(arr) BS_VEC3(arr)
#define BS_V4(arr) BS_VEC4(arr)

/* --- COORDS -> VECTOR --- */
// COORDS -> VECTOR
#define BS_CVEC2(x, y) (bs_vec2){ x, y }
#define BS_CVEC3(x, y, z) (bs_vec3){ x, y, z }
#define BS_CVEC4(x, y, z, w) (bs_vec4) { x, y, z, w }
#define BS_CV2(x, y) BS_CVEC2(x, y)
#define BS_CV3(x, y, z) BS_CVEC3(x, y, z)
#define BS_CV4(x, y, z, w) BS_CVEC4(x, y, z, w)

// COORDS -> VECTOR (INT)
#define BS_CIVEC2(x, y) (bs_ivec2){ x, y }
#define BS_CIVEC3(x, y, z) (bs_ivec3){ x, y, z }
#define BS_CIVEC4(x, y, z, w) (bs_ivec4) { x, y, z, w }
#define BS_CIV2(x, y) BS_CIVEC2(x, y)
#define BS_CIV3(x, y, z) BS_CIVEC3(x, y, z)
#define BS_CIV4(x, y, z, w) BS_CIVEC4(x, y, z, w)

typedef int64_t bs_I64, bs_long;
typedef int32_t bs_I32, bs_int;
typedef int16_t bs_I16, bs_short;
typedef int8_t  bs_I8 , bs_byte;

typedef uint64_t bs_U64, bs_ulong;
typedef uint32_t bs_U32, bs_uint;
typedef uint16_t bs_U16, bs_ushort;
typedef uint8_t  bs_U8 , bs_ubyte;

/* --- UNSIGNED INT TYPES --- */
typedef struct {
	unsigned int x;
	unsigned int y;
} bs_uivec2;

typedef struct {
	unsigned int x;
	unsigned int y;
	unsigned int z;
} bs_uivec3;

typedef struct {
	unsigned int x;
	unsigned int y;
	unsigned int z;
	unsigned int w;
} bs_uivec4;

/* --- TEXTURES --- */
typedef struct {
    bs_ivec3 frame;

    int w, h;
    float texw, texh;

    int type;
    unsigned int id;
    unsigned int unit;
    unsigned char *data;
} bs_Tex2D;

/* --- SHADERS --- */
// TODO: Make #define instead
typedef enum {
    UNIFORM_PROJ,
    UNIFORM_VIEW,

    BS_UNIFORM_TYPE_COUNT,
} BS_UNIFORM_TYPES;

typedef enum {
    BS_POS,
    BS_TEX,
    BS_COL,
    BS_NOR,
    BS_BID,
    BS_WEI,
    BS_V4_,

    BS_MAX_ATTRIB_COUNT
} Bs_ATTRIB_TYPES;

// TODO: Remove this shit
typedef struct {
    bool is_valid;
    int loc;
} bs_Uniform;

typedef struct {
    bs_Uniform uniforms[BS_UNIFORM_TYPE_COUNT];
    int attribs;
    uint8_t attrib_sizes[BS_MAX_ATTRIB_COUNT];
    int attrib_count;

    // OpenGL Variables
    unsigned int id;
    unsigned int vs_id;
    unsigned int fs_id;
    unsigned int gs_id;
} bs_Shader;

typedef struct {
    unsigned int id;
    unsigned int cs_id;
    
    bs_Tex2D *tex;
} bs_ComputeShader;

typedef struct {
    unsigned int id;
    unsigned int block_size;
} bs_UniformBuffer;

typedef struct {
    bs_mat4 view;
    bs_mat4 proj;
} bs_Camera;

typedef struct {
    unsigned int vertex;
    unsigned int index;
} bs_Offset;

typedef struct {
    bs_ivec2 dim;

    unsigned int FBO, RBO;
    int buffer_count;

    int clear;
    int culling;

    int buf_count;
    int buf_alloc;
    bs_Tex2D *bufs;
} bs_Framebuf;

// Contains all objects queued to render the next frame (unless using multiple batches)
typedef struct {
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
} bs_Batch;

typedef struct bs_Joint bs_Joint;
typedef struct {
    bs_mat4 *matrices;

    int joint_count;
    int frame_count;
    int frame_offset_shader;
} bs_Anim;

typedef struct {
    bs_RGBA col;
    bs_Tex2D *tex;

    float metallic;
} bs_Material;

struct bs_Joint {
    // "mat" needs to be the first variable
    bs_mat4 mat;
    bs_mat4 local_inv;
    bs_mat4 bind_matrix;
    bs_mat4 bind_matrix_inv;

    bs_Joint *parent;
    int loc;
};

typedef struct {
    bs_Material material;

    struct {
	bs_vec3 position;
	bs_vec2 tex_coord;
	bs_vec3 normal;
	bs_RGBA color;
	bs_ivec4 bone_ids;
	bs_vec4 weights;
	int unique_index;
    } *vertices;
    int vertex_count;

    int *indices;
    int index_count;
} bs_Prim;

typedef struct {
    char *name; /* Can be freed */
    int vertex_count;

    bs_vec3 pos;
    bs_vec4 rot;
    bs_vec3 sca;
    bs_mat4 mat;

    bs_Prim *prims;
    int prim_count;

    bs_Joint *joints;
    int joint_count;
} bs_Mesh;

typedef struct {
    char *name;

    bs_Mesh *meshes;
    bs_Tex2D *textures;

    int mesh_count;
    int vertex_count;
    int index_count;
} bs_Model;

/* --- VECTOR CONSTANTS --- */
// bs_vec2 Constants
#define bs_vec2_0 (bs_vec2){ 0.0, 0.0 }
#define bs_vec2_1 (bs_vec2){ 1.0, 1.0 }
// bs_vec3 Constants
#define bs_vec3_0 (bs_vec3){ 0.0, 0.0, 0.0 }
#define bs_vec3_1 (bs_vec3){ 1.0, 1.0, 1.0 }
// bs_vec4 Constants
#define bs_vec4_0 (bs_vec4){ 0.0, 0.0, 0.0, 0.0 }
#define bs_vec4_1 (bs_vec4){ 1.0, 1.0, 1.0, 1.0 }

#define bs_ivec4_0 (bs_ivec4){ 0.0, 0.0, 0.0, 0.0 }

/* --- COLOR CONSTANTS --- */
// bs_RGBA Constants
#define bs_RGBA_0 (bs_RGBA){ 0, 0, 0, 0 }
#define bs_RGBA_BLACK (bs_RGBA){ 0, 0, 0, 255 }
#define bs_RGBA_WHITE (bs_RGBA){ 255, 255, 255, 255 }

#define bs_RGBA_RED     (bs_RGBA){ 255, 0  , 0  , 255 }
#define bs_RGBA_GREEN   (bs_RGBA){ 0  , 255, 0  , 255 }
#define bs_RGBA_BLUE    (bs_RGBA){ 0  , 0  , 255, 255 }

#define bs_RGBA_CYAN    (bs_RGBA){ 0  , 255, 255, 255 }
#define bs_RGBA_YELLOW  (bs_RGBA){ 255, 255, 0  , 255 }
#define bs_RGBA_MAGENTA (bs_RGBA){ 255, 0  , 255, 255 }

#define bs_RGBA255_0

#endif /* BS_TYPES_H */
