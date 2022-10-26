#ifndef BS_SHADERS_H
#define BS_SHADERS_H

#include <stdbool.h>
#include <bs_types.h>

/* --- INITIALIZATION --- */
void bs_shaderReplaceAlloc(int amount); 
void bs_replaceInAllShaders(char *old_str, char *new_str);
void bs_freeReplaceBlock();

void bs_shaderMem(char *vs_code, char *fs_code, char *gs_code, bs_Shader *shader);
void bs_shader(char *vs_path, char *fs_path, char *gs_path, bs_Shader *shader);
void bs_loadMemComputeShader(char *cs_code, bs_ComputeShader *compute_shader, bs_Texture *tex);
void bs_loadComputeShader(char *cs_path, bs_ComputeShader *compute_shader, bs_Texture *tex);

// Deprecated
#define bs_loadShader bs_shader
#define bs_loadMemShader bs_shaderMem

/* --- COMPUTE SHADERS --- */
int bs_uniformLoc(int id, char *name);
void bs_switchShader(int id);
void bs_switchShaderCompute(int id);

/* --- UNIFORM BUFFERS --- */
bs_UniformBuffer bs_initUniformBlock(int block_size, int bind_point);
void bs_setUniformBlockDataRange(bs_UniformBuffer buf, void *block, int start, int end);
void bs_setUniformBlockData(bs_UniformBuffer buf, void *block);

/* --- SSBO --- */
bs_U32 bs_SSBO(void *data, int size, int bind_point);
void bs_selectSSBO(bs_U32 ssbo_id);
void bs_pushSSBO(void *data, int offset, int size);

void bs_uniform_mat4(int loc, bs_mat4 mat);
void bs_uniform_float(int loc, float val);
void bs_uniform_int(int loc, int val);

void bs_uniform_vec2(int loc, bs_vec2 vec);
void bs_uniform_ivec2(int loc, bs_ivec2 vec);

void bs_uniform_vec3(int loc, bs_vec3 vec);
void bs_uniform_ivec3(int loc, bs_ivec3 vec);

void bs_uniform_vec4(int loc, bs_vec4 vec);
void bs_uniform_ivec4(int loc, bs_ivec4 vec);

void bs_setMemBarrier(int barrier);
void bs_dispatchComputeShader(int x, int y, int z);

#endif /* BS_SHADERS_H */
