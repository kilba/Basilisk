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
	// The order in which the shader was loaded (starts at 0)
	int index;

	bs_Uniform uniforms[UNIFORM_TYPE_COUNT];

	// OpenGL Variables
	unsigned int id;
	unsigned int vs_id;
	unsigned int fs_id;
	unsigned int gs_id;
} bs_Shader;

// INITIALIZATION
bs_Shader bs_loadMemShader(char *vs_code, char *fs_code, char *gs_code);
bs_Shader bs_loadShader(char *vs_path, char *fs_path, char *gs_path);

void bs_setShaderAtlas(bs_Shader *shader, bs_Atlas *atlas, char *uniform_name);

// SETTING DEFAULT UNIFORMS
void bs_setTimeUniform(bs_Shader *shader, float time);
void bs_setViewMatrixUniform(bs_Shader *shader, void *cam);
void bs_setProjMatrixUniform(bs_Shader *shader, void *cam);

// SHADER ABSTRACTION LAYER
int bs_getUniformLoc(bs_Shader *shader, char *name);
void bs_switchShader(bs_Shader *shader);

void bs_uniform_mat4(int loc, float mat[4][4]);
void bs_uniform_float(int loc, float val);

#endif /* BS_SHADERS_H */