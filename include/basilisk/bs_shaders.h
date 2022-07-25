#ifndef BS_SHADERS_H
#define BS_SHADERS_H

#include <stdbool.h>
#include <bs_textures.h>

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
	bs_Texture *tex;
	unsigned int id;
	unsigned int cs_id;
} bs_ComputeShader;

// INITIALIZATION
void bs_loadMemShader(char *vs_code, char *fs_code, char *gs_code, bs_Shader *shader);
void bs_loadShader(char *vs_path, char *fs_path, char *gs_path, bs_Shader *shader);
void bs_loadMemComputeShader(char *cs_code, bs_ComputeShader *compute_shader, bs_Texture *tex);
void bs_loadComputeShader(char *cs_path, bs_ComputeShader *compute_shader, bs_Texture *tex);

void bs_setShaderAtlas(bs_Shader *shader, bs_Atlas *atlas, char *uniform_name);

// SETTING DEFAULT UNIFORMS
void bs_setTimeUniform(bs_Shader *shader, float time);
void bs_setViewMatrixUniform(bs_Shader *shader, void *cam);
void bs_setProjMatrixUniform(bs_Shader *shader, void *cam);

// SHADER ABSTRACTION LAYER
void bs_getUniformLoc(bs_Shader *shader, char *name, int *result);
void bs_switchShader(bs_Shader *shader);
void bs_switchShaderCompute(bs_ComputeShader *compute_shader);

void bs_uniform_mat4(int loc, float mat[4][4]);
void bs_uniform_float(int loc, float val);

#endif /* BS_SHADERS_H */