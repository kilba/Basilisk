#ifndef BS_TYPES_H
#define BS_TYPES_H

/* --- TEXTURES --- */
typedef struct {
    int w, h;
    unsigned int id;
    unsigned char *data;
} bs_Tex2D;

typedef struct {
    unsigned int w, h;
    unsigned int x, y;
    float tex_x, tex_y;
    float tex_wx, tex_hy;
    unsigned char *data;
} bs_AtlasSlice;

typedef struct {
    bs_Tex2D tex;
    bs_AtlasSlice *textures;

    int tex_count;
} bs_Atlas;

/* --- SHADERS --- */
typedef enum {
	UNIFORM_PROJ,
	UNIFORM_VIEW,
	UNIFORM_TIME,

	UNIFORM_TYPE_COUNT,
} bs_STDUniforms;

typedef struct {
	bool is_valid;
	int loc;
} bs_Uniform;

typedef struct {
	// The order in which the shader was loaded (zero-indexed)
	int index;

	bs_Uniform uniforms[UNIFORM_TYPE_COUNT];

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

/* --- GENERAL --- */
typedef enum {
	bs_WND_DEFAULT = 0,

	bs_WND_TRANSPARENT = 1,
	bs_WND_NO_TITLE_BAR = 2,
	bs_WND_TOPMOST = 4,
	bs_WND_INVISIBLE = 8,
	bs_WND_UNCLICKABLE = 16,
} bs_WNDSettings;

typedef mat2 bs_mat2;
typedef mat3 bs_mat3;
typedef mat4 bs_mat4;

typedef versor bs_quat;

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
} bs_RGBA;

/* --- FLOAT TYPES --- */
typedef struct {
	float r;
	float g;
	float b;
	float a;
} bs_fRGBA;

typedef struct {
	float x;
	float y;
} bs_vec2;

typedef struct {
	float x;
	float y;
	float z;
} bs_vec3;

typedef struct {
	float x;
	float y;
	float z;
	float w;
} bs_vec4;

/* --- INT TYPES --- */
typedef struct {
	int x;
	int y;
} bs_ivec2;

typedef struct {
	int x;
	int y;
	int z;
} bs_ivec3;

typedef struct {
	int x;
	int y;
	int z;
	int w;
} bs_ivec4;

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

typedef struct {
	bs_mat4 view;
	bs_mat4 proj;
	bs_vec3 pos;
	bs_vec2 res;
} bs_Camera;

typedef struct {
	bs_vec3 position;
	bs_vec2 tex_coord;
	bs_vec3 normal;
	bs_RGBA color;
} bs_Vertex;

// Vertex declaration for rigged models
typedef struct {
	bs_vec3 position;
	bs_vec2 tex_coord;
	bs_vec3 normal;
	bs_RGBA color;
	bs_ivec4 bone_ids;
	bs_vec4 weights;
} bs_RVertex;

typedef struct {
	int render_width;
	int render_height;

	bs_Shader *shader;
	void (*render)();

	unsigned int VAO, VBO;

	unsigned int FBO, RBO;
	unsigned int texture_color_buffer;
} bs_Framebuffer;

// Contains all objects queued to render the next frame (unless using multiple batches)
typedef struct {
	bs_Shader *shader;
	bs_Camera *camera;

	void *vertices;
	int *indices;

	int draw_mode;
	int vertex_draw_count;
	int index_draw_count;

	int types;
	int attrib_count;
	int attrib_size_bytes;
	size_t attrib_offset;

	unsigned int VAO, VBO, EBO;
} bs_Batch;

typedef struct bs_Joint bs_Joint;
typedef struct {
	bs_Joint *joints;
	int joint_count;
} bs_Anim;

typedef struct {
	bs_RGBA base_color;
	bs_AtlasSlice *tex;

	bs_vec3 specular;
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
	int attrib_count;

	bs_RVertex *vertices;
	int vertex_count;

	int *indices;
	int index_count;
} bs_Prim;

typedef struct {
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
	bs_Mesh *meshes;
	bs_AtlasSlice **textures;

	int mesh_count;
	int vertex_count;
	int index_count;
} bs_Model;

#endif /* BS_TYPES_H */