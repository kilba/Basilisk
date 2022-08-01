#ifndef BS_SHADERS_H
#define BS_SHADERS_H

#include <stdbool.h>
#include <bs_types.h>

// INITIALIZATION
void bs_loadMemShader(char *vs_code, char *fs_code, char *gs_code, bs_Shader *shader);
void bs_loadShader(char *vs_path, char *fs_path, char *gs_path, bs_Shader *shader);
void bs_loadMemComputeShader(char *cs_code, bs_ComputeShader *compute_shader, bs_Tex2D *tex);
void bs_loadComputeShader(char *cs_path, bs_ComputeShader *compute_shader, bs_Tex2D *tex);

void bs_setShaderAtlas(bs_Shader *shader, bs_Atlas *atlas, char *uniform_name);

// SETTING DEFAULT UNIFORMS
void bs_setTimeUniform(bs_Shader *shader, float time);
void bs_setViewMatrixUniform(bs_Shader *shader, void *cam);
void bs_setProjMatrixUniform(bs_Shader *shader, void *cam);

// SHADER ABSTRACTION LAYER
void bs_getUniformLoc(int id, char *name, int *result);
void bs_switchShader(int id);
void bs_switchShaderCompute(int id);

void bs_uniform_mat4(int loc, float mat[4][4]);
void bs_uniform_float(int loc, float val);
void bs_uniform_vec3(int loc, bs_vec3 vec);

void bs_setMemBarrier(int barrier);
void bs_dispatchComputeShader(int x, int y, int z);

#endif /* BS_SHADERS_H */