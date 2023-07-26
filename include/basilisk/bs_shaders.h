#ifndef BS_SHADERS_H
#define BS_SHADERS_H

#include <stdbool.h>
#include <bs_types.h>

enum {
    BS_IDXS,
    BS_MODEL,
    BS_MATERIAL,
    BS_ANIM,

    BS_BUF_COUNT
};

enum {
    BS_SSBO_X,
    BS_SSBO_ANIMS,
    BS_SSBO_IDXS,
    BS_SSBO_MODELS,
    BS_SSBO_MATERIALS,
};

/* --- SHADER BUFFERS --- */
// Bufs
bs_U32 bs_shaderModel();
bs_U32 bs_shaderModelInit(bs_mat4 model);
bs_U32 bs_shaderMaterial();
bs_U32 bs_shaderMaterialInit(bs_Material *material);

// References
void bs_updateShaderReferences(bs_Idxs idxs, size_t offset);
bs_Refs bs_shaderReferences(bs_Idxs idxs);
bs_Refs bs_shaderModelReferences(bs_Model *model, bs_Idxs unified);

void bs_updateShaderFrame(bs_U32 frame, bs_Refs refs);
void bs_updateShaderModel(bs_mat4 mat, bs_U32 offset);
void bs_updateShaderMaterial(bs_Material *material, bs_U32 offset);

/* --- INITIALIZATION --- */
void bs_shaderBufs();
void bs_shaderReplaceAlloc(int amount); 
void bs_replaceInAllShaders(char *old_str, char *new_str);
void bs_freeReplaceBlock();

/* --- COMPUTE SHADERS --- */
void bs_shaderRange(bs_Shader *shaders, int num_vs, char **vs_paths, const char *fs_path, const char *gs_path);
void bs_shader(bs_Shader *shader, bs_VertexShader *vs, bs_U32 fs, bs_U32 gs);
void bs_vertexShaderMem(bs_VertexShader *vs, char *code);
void bs_fragmentShaderMem(bs_U32 *fs, char *code);
void bs_geometryShaderMem(bs_U32 *gs, char *code);
void bs_vertexShader(bs_VertexShader *vs, const char *path);
void bs_fragmentShader(bs_U32 *fs, const char *path);
void bs_geometryShader(bs_U32 *gs, const char *path);
void bs_cShaderMem(bs_ComputeShader *shader, const char *cs_code, bs_Texture *tex);
void bs_cShaderMem(bs_ComputeShader *shader, const char *cs_path, bs_Texture *tex);

int bs_uniformLoc(bs_Shader *shader, const char *name);
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

void bs_uniformM4(int loc, bs_mat4 mat);
void bs_uniformFloat(int loc, float val);
void bs_uniformI32(int loc, bs_I32 val);
void bs_uniformU32(int loc, bs_U32 val);

void bs_uniformV2(int loc, bs_vec2 vec);
void bs_uniformIV2(int loc, bs_ivec2 vec);

void bs_uniformV3(int loc, bs_vec3 vec);
void bs_uniformIV3(int loc, bs_ivec3 vec);

void bs_uniformV4(int loc, bs_vec4 vec);
void bs_uniformIV4(int loc, bs_ivec4 vec);

void bs_setMemBarrier(int barrier);
void bs_dispatchComputeShader(int x, int y, int z);

#endif /* BS_SHADERS_H */
