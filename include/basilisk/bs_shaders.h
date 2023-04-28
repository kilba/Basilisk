#ifndef BS_SHADERS_H
#define BS_SHADERS_H

#include <stdbool.h>
#include <bs_types.h>

enum {
    BS_MODEL_IDX_OFFSET,
    BS_MATERIAL_IDX_OFFSET,
    BS_TEXTURE_IDX_OFFSET,
    BS_ANIM_IDX_OFFSET,

    BS_SHADER_IDX_COUNT
};

enum {
    BS_SSBO_X,
    BS_SSBO_ANIMS,
    BS_SSBO_IDXS,
    BS_SSBO_MODELS
};

/* --- SHADER BUFFERS --- */
bs_U32 bs_shaderModel(bs_mat4 mat);
void bs_updateShaderModel(bs_mat4 mat, bs_U32 offset);

/* --- INITIALIZATION --- */
void bs_shaderBufs();
void bs_shaderReplaceAlloc(int amount); 
void bs_replaceInAllShaders(char *old_str, char *new_str);
void bs_freeReplaceBlock();

void bs_shaderMem(bs_Shader *shader, const char *vs_code, const char *fs_code, const char *gs_code);
void bs_shader(bs_Shader *shader, const char *vs_path, const char *fs_path, const char *gs_path);
void bs_cShaderMem(bs_ComputeShader *shader, const char *cs_code, bs_Texture *tex);
void bs_cShaderMem(bs_ComputeShader *shader, const char *cs_path, bs_Texture *tex);

/* --- COMPUTE SHADERS --- */
int bs_uniformLoc(int id, const char *name);
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
