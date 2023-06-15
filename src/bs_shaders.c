// GL
#include <glad/glad.h>
#include <cglm/cglm.h>

// Basilisk
#include <bs_mem.h>
#include <bs_core.h>
#include <bs_debug.h>
#include <bs_shaders.h>
#include <bs_textures.h>

// STD
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

bs_U32 curr_ssbo = 0;

typedef struct ReplaceBuf ReplaceBuf;
struct ReplaceBuf {
    char *old_str;
    char *new_str;
} *replace_buf = NULL;
// TODO: Make static
int replace_buf_size = 0;
int replace_buf_curr = 0;

int num_shader_indices = 1024 * 8;

bs_U32 shader_SSBOs[BS_BUF_COUNT];
bs_U32 shader_buf_count[BS_BUF_COUNT] = { 0 };

char *global_shader = NULL;

const char *last_vs_path = NULL, *last_fs_path = NULL, *last_gs_path = NULL;

/* --- UPDATE BUFFERS --- */
void bs_updateShaderBuf(const bs_U32 type, void *data, bs_U32 offset, bs_U32 size) {
    bs_selectSSBO(shader_SSBOs[type]);
    bs_pushSSBO(data, offset, size);
}

void bs_updateShaderModel(bs_mat4 mat, bs_U32 offset) {
    bs_updateShaderBuf(BS_MODEL, mat.a, offset * sizeof(bs_mat4), sizeof(bs_mat4));
}

void bs_updateShaderMaterial(bs_Material *material, bs_U32 offset) {
    bs_v4print(material->data.color);
    bs_updateShaderBuf(BS_MATERIAL, &material->data, offset * sizeof(bs_ShaderMaterial), sizeof(bs_ShaderMaterial));
}

/* --- CREATE BUFFERS --- */
bs_U32 bs_shaderModel() {
    return shader_buf_count[BS_MODEL]++;
}

bs_U32 bs_shaderModelInit(bs_mat4 model) {
    bs_updateShaderModel(model, shader_buf_count[BS_MODEL]);
    return shader_buf_count[BS_MODEL]++;
}

bs_U32 bs_shaderMaterial() {
    return shader_buf_count[BS_MATERIAL]++;
}

bs_U32 bs_shaderMaterialInit(bs_Material *material) {
    bs_updateShaderMaterial(material, shader_buf_count[BS_MATERIAL]);
    return shader_buf_count[BS_MATERIAL]++;
}

/* --- UPDATE REFERENCES --- */
void bs_updateShaderReferences(bs_Idxs idxs, size_t offset) {
    bs_selectSSBO(shader_SSBOs[BS_IDXS]);
    bs_pushSSBO(&idxs, offset, sizeof(bs_Idxs));
}

void bs_updateShaderReference(void *value, size_t size, size_t offset) {
    bs_selectSSBO(shader_SSBOs[BS_IDXS]);
    bs_pushSSBO(value, offset, size);
}

/* --- CREATE REFERENCES --- */
bs_Refs bs_shaderModelReferences(bs_Model *model, bs_Idxs unified) {
    bs_U32 offset = shader_buf_count[BS_IDXS];

    for(int i = 0; i < model->material_count; i++) {
	bs_Material *mat = model->materials + i;
	unified.texture_handle = mat->texture_handle;
	unified.material = mat->shader_material;
	bs_shaderReferences(unified);
    }

    return (bs_Refs) { offset, model->material_count };
}

bs_Refs bs_shaderReferences(bs_Idxs idxs) {
    int test = shader_buf_count[BS_IDXS];
    bs_updateShaderReferences(idxs, shader_buf_count[BS_IDXS] * sizeof(bs_Idxs));
    shader_buf_count[BS_IDXS]++;
    return (bs_Refs){ test, 1 };
}

// Shader frames treated as a reference and not buffer since it's only an INT
void bs_updateShaderFrame(bs_U32 frame, bs_Refs refs) {
    // TODO: Update in one go
    for(int i = 0; i < refs.count; i++) {
        bs_updateShaderReference(&frame, sizeof(bs_U32), (refs.value + i) * sizeof(bs_Idxs) + offsetof(bs_Idxs, frame));
    }
}

void bs_shaderBufs() {
    int err, len;
    global_shader = bs_fileContents("basilisk.bsh", &len, &err);

    if(global_shader != NULL && err == 0)
	bs_replaceInAllShaders("#define BASILISK", global_shader);

    shader_SSBOs[BS_IDXS]     = bs_SSBO(NULL, num_shader_indices * sizeof(bs_U32) * BS_BUF_COUNT, BS_SSBO_IDXS + 1);
    shader_SSBOs[BS_MODEL]    = bs_SSBO(NULL, BS_MODEL_INCR_BY * sizeof(bs_mat4), BS_SSBO_MODELS + 1);
    shader_SSBOs[BS_MATERIAL] = bs_SSBO(NULL, BS_MATERIAL_INCR_BY * sizeof(bs_ShaderMaterial), BS_SSBO_MATERIALS + 1); // 32 MB
}

/* Not necessary, but prevents multiple calls to realloc() */
void bs_shaderReplaceAlloc(int amount) {
    replace_buf = realloc(replace_buf, amount * sizeof(ReplaceBuf));
    replace_buf_size = amount;
}

void bs_replaceInAllShaders(char *old_str, char *new_str) {
    if(replace_buf_curr >= replace_buf_size)
	bs_shaderReplaceAlloc(replace_buf_size+1);

    //replace_buf[replace_buf_curr].old_str = malloc(1+strlen(old_str));
    //replace_buf[replace_buf_curr].new_str = malloc(1+strlen(new_str));
    //strcpy(replace_buf[replace_buf_curr].old_str, old_str);
    //strcpy(replace_buf[replace_buf_curr].new_str, new_str);

    replace_buf[replace_buf_curr].old_str = old_str;
    replace_buf[replace_buf_curr].new_str = new_str;
    replace_buf_curr++;
}

void bs_freeReplaceBlock() {
    free(replace_buf);
}

// Gets all default uniform locations
void bs_setDefShaderUniforms(bs_Shader *shader, const char *shader_code){
    const char *def_uniforms[] = { 
        "bs_Proj", 
        "bs_View", 
    };

    // Loop through all the uniform types
    for (int i = 0; i < BS_UNIFORM_TYPE_COUNT; i++) {
        // Check if the shader contains the uniform
        if(strstr(shader_code, def_uniforms[i])){
            bs_Uniform *uniform = &shader->uniforms[i];

            // If uniform already has been set
            if(uniform->is_valid)
                continue;

            int uniform_loc = glGetUniformLocation(shader->id, def_uniforms[i]);
            // If uniform is unused or non existent
            if(uniform_loc == -1)
                continue;

            uniform->is_valid = true;
            uniform->loc = uniform_loc;
        }
    }

    // Set all the texture units
    int texture_unit_count = 0;
    char uni_texture[] = "bs_Texture0\0\0\0";
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_unit_count);
    for(int i = 0; i < texture_unit_count; i++) {
        char buffer[3] = "\0\0\0";
        int loc;

        itoa(i, buffer, 10);
        memcpy(uni_texture+10, buffer, 3);

        loc = bs_uniformLoc(shader, uni_texture);
        if(loc == -1)
            continue;

        bs_uniformI32(loc, i);
    }
}

void bs_setDefShaderAttribs(bs_Shader *shader, const char *vs_code) {
    struct {
	char *name;
	int value;
	int size;
    } attribs[] = {
	{ "in vec3 bs_Pos" , BS_VAL_POS, sizeof(bs_vec3) },
	{ "in vec2 bs_Tex" , BS_VAL_TEX, sizeof(bs_vec2) },
	{ "in vec4 bs_Col" , BS_VAL_COL, sizeof(bs_RGBA) },
	{ "in vec3 bs_Nor" , BS_VAL_NOR, sizeof(bs_vec3) },
	{ "in ivec4 bs_BID", BS_VAL_BID, sizeof(bs_ivec4) },
	{ "in vec4 bs_Wei" , BS_VAL_WEI, sizeof(bs_vec4) },
	{ "in uint bs_Ref" , BS_VAL_REF, sizeof(int) },
	{ "in vec4 bs_V4_" , BS_VAL_V4_, sizeof(bs_vec4) },
	{ "in float bs_V1_", BS_VAL_V1_, sizeof(float) },
    };

    for(int i = 0; i < BS_MAX_ATTRIB_COUNT; i++) {
	shader->attrib_sizes[i] = 0;
	if(strstr(vs_code, attribs[i].name)) {
	    shader->attrib_size_bytes += attribs[i].size;
	    shader->attrib_sizes[i] = attribs[i].size;
	    shader->attribs |= attribs[i].value;
	    shader->attrib_count++;
	}
    }
}

void bs_shaderErrorCheck(GLuint *shader, int shadertype) {
    GLint isCompiled = 0;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &maxLength);

        GLchar errorLog[100];
        glGetShaderInfoLog(*shader, maxLength, &maxLength, &errorLog[0]);

	if(last_vs_path != NULL) {
	    printf("ERROR IN: ");
	    switch(shadertype) {
		case GL_VERTEX_SHADER: printf("%s\n", last_vs_path); break;
		case GL_FRAGMENT_SHADER: printf("%s\n", last_fs_path); break;
		case GL_GEOMETRY_SHADER: printf("%s\n", last_gs_path); break;
		default: break;
	    }
	}

        bs_print(BS_CLE, "\n");
        bs_print(BS_CLE, errorLog);

        glDeleteShader(*shader);
    }
}

const char *bs_replaceInShader(const char *code) {
    const char *new_code = code;
    for(int i = 0; i < replace_buf_curr; i++) {
	ReplaceBuf *buf = replace_buf + i;
	char *repl = bs_replaceFirstSubstring(new_code, buf->old_str, buf->new_str);
	if(repl != NULL)
	    new_code = repl;
    }
    return new_code;
}

void bs_loadShaderCode(int program, GLuint *shader_id, const char *shader_code, int type) {
    const GLchar *replaced_shader_code = bs_replaceInShader(shader_code);

    *shader_id = glCreateShader(type);
    glShaderSource(*shader_id, 1, &replaced_shader_code, NULL);

    glCompileShader(*shader_id);
    bs_shaderErrorCheck(shader_id, type);
    glAttachShader(program, *shader_id);

    if((char*)replaced_shader_code != shader_code)
	free((char*)replaced_shader_code);
}

void bs_setDefaultUniformLocations(bs_Shader *shader, const char *vs_code, const char *fs_code, const char *gs_code) {
    for(int i = 0; i < BS_UNIFORM_TYPE_COUNT; i++)
        shader->uniforms[i].is_valid = false;

    bs_setDefShaderUniforms(shader, vs_code);
    bs_setDefShaderUniforms(shader, fs_code);

    bs_setDefShaderAttribs(shader, vs_code);

    if(gs_code != 0)
        bs_setDefShaderUniforms(shader, gs_code);
}

void bs_shaderMem(bs_Shader *shader, const char *vs_code, const char *fs_code, const char *gs_code) {
    if(vs_code == NULL || fs_code == NULL) {
        shader->id = -1;
        return;
    }

    shader->attribs = 0;
    shader->attrib_count = 0;
    shader->attrib_size_bytes = 0;
    shader->id = glCreateProgram();

    bs_U32 vs_id, fs_id, gs_id;

    bs_loadShaderCode(shader->id, &vs_id, vs_code, GL_VERTEX_SHADER);
    bs_loadShaderCode(shader->id, &fs_id, fs_code, GL_FRAGMENT_SHADER);

    // Geometry shader is not mandatory
    if(gs_code != 0)
        bs_loadShaderCode(shader->id, &gs_id, gs_code, GL_GEOMETRY_SHADER);

    glLinkProgram(shader->id);
    glUseProgram(shader->id);
    
    bs_setDefaultUniformLocations(shader, vs_code, fs_code, gs_code);

    // Free
    glDetachShader(shader->id, vs_id);
    glDetachShader(shader->id, fs_id);
    if(gs_code != 0)
	glDetachShader(shader->id, gs_id);

    glDeleteShader(shader->id);
    return;
}

void bs_shader(bs_Shader *shader, const char *vs_path, const char *fs_path, const char *gs_path) {
    int vs_err_code;
    int fs_err_code;
    int gs_err_code;

    last_vs_path = vs_path;
    last_fs_path = fs_path;
    last_gs_path = gs_path;

    // Load shader source code into memory
    int len;
    char *vscode = bs_fileContents(vs_path, &len, &vs_err_code);
    char *fscode = bs_fileContents(fs_path, &len, &fs_err_code);
    char *gscode = bs_fileContents(gs_path, &len, &gs_err_code);

    // Don't compile shaders if file wasn't found
    if(vs_err_code == 2 || fs_err_code == 2) {
        shader->id = -1;
        return;
    }

    // Load the shader from memory, compile and return it
    bs_shaderMem(shader, vscode, fscode, gscode);
    free(vscode);
    free(fscode);
    free(gscode);
}

/* COMPUTE SHADERS */
void bs_loadMemComputeShader(bs_ComputeShader *shader, const char *cs_code, bs_Texture *tex) {
    if(cs_code == NULL)
        return;

    bs_loadShaderCode(shader->id, &shader->cs_id, cs_code, GL_COMPUTE_SHADER);

    shader->id = glCreateProgram();
    glLinkProgram(shader->id);
    glUseProgram(shader->id);

    // TODO: Check if texture is still bound
    shader->tex = tex;
    glBindImageTexture(0, shader->tex->id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}

void bs_loadComputeShader(bs_ComputeShader *shader, const char *cs_path, bs_Texture *tex) {
    int cs_err_code;
    int len;

    char *cscode = bs_fileContents(cs_path, &len, &cs_err_code);
    bs_loadMemComputeShader(shader, cscode, tex);
    free(cscode);
}

void bs_setMemBarrier(int barrier) {
    glMemoryBarrier(barrier);
}

void bs_dispatchComputeShader(int x, int y, int z) {
    glDispatchCompute(x, y, z);
}

/* --- UNIFORM BLOCKS --- */
bs_UniformBuffer bs_initUniformBlock(int block_size, int bind_point) {
    bs_UniformBuffer ubo;
    ubo.block_size = block_size;

    glGenBuffers(1, &ubo.id);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo.id);
    glBufferData(GL_UNIFORM_BUFFER, block_size, NULL, GL_STREAM_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, bind_point, ubo.id); 

    return ubo;
}

void bs_setUniformBlockDataRange(bs_UniformBuffer buf, void *block, int start, int end) {
    glBindBuffer(GL_UNIFORM_BUFFER, buf.id);
    glBufferSubData(GL_UNIFORM_BUFFER, start, end, block); 
}

void bs_setUniformBlockData(bs_UniformBuffer buf, void *block) {
    bs_setUniformBlockDataRange(buf, block, 0, buf.block_size);
}

/* --- SSBO --- */
bs_U32 bs_SSBO(void *data, int size, int bind_point) {
    bs_U32 ssbo;

    glGenBuffers(1, &ssbo);
    bs_selectSSBO(ssbo);

    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bind_point, ssbo);

    return ssbo;
}

void bs_selectSSBO(bs_U32 ssbo_id) {
    if(curr_ssbo == ssbo_id)
	return;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
    curr_ssbo = ssbo_id;
}

void bs_pushSSBO(void *data, int offset, int size) {
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
}

int bs_uniformLoc(bs_Shader *shader, const char *name) {
    return glGetUniformLocation(shader->id, name);
}

void bs_switchShader(int id) {
    glUseProgram(id);
}

void bs_switchShaderCompute(int id) {
    glUseProgram(id);
}

// MATRICES
// TODO: mat2, mat3
void bs_uniformM4(int loc, bs_mat4 mat) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, (float *)mat.a);
}

// SCALARS
// TODO: bool, double
void bs_uniformFloat(int loc, float val) {
    glUniform1f(loc, val);
}

void bs_uniformI32(int loc, bs_I32 val) {
    glUniform1i(loc, val);
}

void bs_uniformU32(int loc, bs_U32 val) {
    glUniform1ui(loc, val);
}

// VECTORS
// TODO: bvecn, ivecn, uvecn, vecn, dvecn

void bs_uniformV2(int loc, bs_vec2 vec) {
    glUniform2f(loc, vec.x, vec.y);
}

void bs_uniformIV2(int loc, bs_ivec2 vec) {
    glUniform2i(loc, vec.x, vec.y);
}

void bs_uniformV3(int loc, bs_vec3 vec) {
    glUniform3f(loc, vec.x, vec.y, vec.z);
}

void bs_uniformIV3(int loc, bs_ivec3 vec) {
    glUniform3i(loc, vec.x, vec.y, vec.z);
}

void bs_uniformV4(int loc, bs_vec4 vec) {
    glUniform4f(loc, vec.x, vec.y, vec.z, vec.w);
}

void bs_uniformIV4(int loc, bs_ivec4 vec) {
    glUniform4i(loc, vec.x, vec.y, vec.z, vec.w);
}

// TODO: Arrays
