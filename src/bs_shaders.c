// GL
#include "bs_types.h"
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
int replace_buf_size = 0;
int replace_buf_curr = 0;

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
void bs_setDefShaderUniforms(bs_Shader *shader, char *shader_code){
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

        loc = bs_uniformLoc(shader->id, uni_texture);
        if(loc == -1)
            continue;
        bs_uniform_int(loc, i);
    }
}

void bs_setDefShaderAttribs(bs_Shader *shader, char *vs_code) {
    const char *def_attribs [] = { 
	"bs_Pos", 
	"bs_TexCoord", 
	"bs_Color", 
	"bs_Normal",
	"bs_Bone_Ids",
	"bs_Weights",
	"bs_Attr_Vec4"
    };

    int values[] = { 
	BS_POSITION,
	BS_TEX_COORD,
	BS_COLOR,
	BS_NORMAL,
	BS_BONE_IDS,
	BS_WEIGHTS,
	BS_ATTR_VEC4,
    };

    uint8_t sizes[] = {
	sizeof(bs_vec3),  /* BS_POS */
	sizeof(bs_vec2),  /* BS_TEX */
	sizeof(bs_RGBA),  /* BS_COL */
	sizeof(bs_vec3),  /* BS_NOR */
	sizeof(bs_ivec4), /* BS_BID */
	sizeof(bs_vec4),  /* BS_WEI */
	sizeof(bs_vec4)   /* BS_V4_ */
    };

    for(int i = 0; i < BS_MAX_ATTRIB_COUNT; i++) {
	shader->attrib_sizes[i] = 0;
	if(strstr(vs_code, def_attribs[i])) {
	    shader->attrib_sizes[i] = sizes[i];
	    shader->attribs |= values[i];
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

        switch(shadertype) {
            case GL_VERTEX_SHADER: bs_print(BS_CLE, "%s", "Vertex Shader Error!"); break;
            case GL_FRAGMENT_SHADER: bs_print(BS_CLE, "%s", "Fragment Shader Error!"); break;
            case GL_GEOMETRY_SHADER: bs_print(BS_CLE, "%s", "Geometry Shader Error!"); break;
            case GL_COMPUTE_SHADER: bs_print(BS_CLE, "%s", "Compute Shader Error!"); break;
        }

        bs_print(BS_CLE, "\n");
        bs_print(BS_CLE, errorLog);

        glDeleteShader(*shader);
    }
}

char *bs_replaceInShader(char *code) {
    char *new_code = code;
    for(int i = 0; i < replace_buf_curr; i++) {
	ReplaceBuf *buf = replace_buf + i;
	char *repl = bs_replaceFirstSubstring(new_code, buf->old_str, buf->new_str);
	if(repl != NULL)
	    new_code = repl;
    }
    return new_code;
}

void bs_loadShaderCode(int program, GLuint *shader_id, char *shader_code, int type) {
    const GLchar *replaced_shader_code = bs_replaceInShader(shader_code);

    *shader_id = glCreateShader(type);
    glShaderSource(*shader_id, 1, &replaced_shader_code, NULL);

    glCompileShader(*shader_id);
    bs_shaderErrorCheck(shader_id, type);
    glAttachShader(program, *shader_id);

    if((char*)replaced_shader_code != shader_code) {
	free((char*)replaced_shader_code);
    }
}

void bs_setDefaultUniformLocations(bs_Shader *shader, char *vs_code, char *fs_code, char *gs_code) {
    for(int i = 0; i < BS_UNIFORM_TYPE_COUNT; i++) {
        shader->uniforms[i].is_valid = false;
    }

    bs_setDefShaderUniforms(shader, (char *)vs_code);
    bs_setDefShaderUniforms(shader, (char *)fs_code);

    bs_setDefShaderAttribs(shader, (char *)vs_code);

    if(gs_code != 0) {
        bs_setDefShaderUniforms(shader, (char *)gs_code);
    }
}

void bs_loadMemShader(char *vs_code, char *fs_code, char *gs_code, bs_Shader *shader) {
    if(vs_code == NULL || fs_code == NULL) {
        shader->id = -1;
        return;
    }

    shader->attribs = 0;
    shader->attrib_count = 0;
    shader->id = glCreateProgram();

    bs_loadShaderCode(shader->id, &shader->vs_id, vs_code, GL_VERTEX_SHADER);
    bs_loadShaderCode(shader->id, &shader->fs_id, fs_code, GL_FRAGMENT_SHADER);

    // Geometry shader is not mandatory
    if(gs_code != 0) {
        bs_loadShaderCode(shader->id, &shader->gs_id, gs_code, GL_GEOMETRY_SHADER);
    }

    glLinkProgram(shader->id);
    glUseProgram(shader->id);

    bs_setDefaultUniformLocations(shader, vs_code, fs_code, gs_code);
    return;
}

void bs_loadShader(char *vs_path, char *fs_path, char *gs_path, bs_Shader *shader) {
    int vs_err_code;
    int fs_err_code;
    int gs_err_code;

    // Load shader source code into memory
    int len;
    char *vscode = bs_readFileToString(vs_path, &len, &vs_err_code);
    char *fscode = bs_readFileToString(fs_path, &len, &fs_err_code);
    char *gscode = bs_readFileToString(gs_path, &len, &gs_err_code);

    // Don't compile shaders if file wasn't found
    if(vs_err_code == 2 || fs_err_code == 2) {
        shader->id = -1;
        return;
    }

    // Load the shader from memory, compile and return it
    bs_loadMemShader(vscode, fscode, gscode, shader);
    free(vscode);
    free(fscode);
    free(gscode);
}

/* COMPUTE SHADERS */
void bs_loadMemComputeShader(char *cs_code, bs_ComputeShader *compute_shader, bs_Tex2D *tex) {
    if(cs_code == NULL)
        return;

    bs_loadShaderCode(compute_shader->id, &compute_shader->cs_id, cs_code, GL_COMPUTE_SHADER);

    compute_shader->id = glCreateProgram();
    glLinkProgram(compute_shader->id);
    glUseProgram(compute_shader->id);

    // TODO: Check if texture is still bound
    compute_shader->tex = tex;
    glBindImageTexture(0, compute_shader->tex->id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}

void bs_loadComputeShader(char *cs_path, bs_ComputeShader *compute_shader, bs_Tex2D *tex) {
    int cs_err_code;
    int len;

    char *cscode = bs_readFileToString(cs_path, &len, &cs_err_code);
    bs_loadMemComputeShader(cscode, compute_shader, tex);
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
    if(bind_point == 0) {
        // bs_print(BS_WAR, 
            // "You have set the uniform block bind point to 0, "
            // "which is also the bind point for the engine's global uniforms."
        // );
    }

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
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
    curr_ssbo = ssbo_id;
}

void bs_pushSSBO(void *data, int offset, int size) {
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
}

int bs_uniformLoc(int id, char *name) {
    return glGetUniformLocation(id, name);
}

void bs_switchShader(int id) {
    glUseProgram(id);
}

void bs_switchShaderCompute(int id) {
    glUseProgram(id);
}

// MATRICES
// TODO: mat2, mat3
void bs_uniform_mat4(int loc, float mat[4][4]) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, mat[0]);
}

// SCALARS
// TODO: bool, int, uint, double
void bs_uniform_float(int loc, float val) {
    glUniform1f(loc, val);
}

void bs_uniform_int(int loc, int val) {
    glUniform1i(loc, val);
}

// VECTORS
// TODO: bvecn, ivecn, uvecn, vecn, dvecn

void bs_uniform_vec2(int loc, bs_vec2 vec) {
    glUniform2f(loc, vec.x, vec.y);
}

void bs_uniform_ivec2(int loc, bs_ivec2 vec) {
    glUniform2i(loc, vec.x, vec.y);
}

void bs_uniform_vec3(int loc, bs_vec3 vec) {
    glUniform3f(loc, vec.x, vec.y, vec.z);
}

void bs_uniform_ivec3(int loc, bs_ivec3 vec) {
    glUniform3i(loc, vec.x, vec.y, vec.z);
}

void bs_uniform_vec4(int loc, bs_vec4 vec) {
    glUniform4f(loc, vec.x, vec.y, vec.z, vec.w);
}

void bs_uniform_ivec4(int loc, bs_ivec4 vec) {
    glUniform4i(loc, vec.x, vec.y, vec.z, vec.w);
}

// TODO: Arrays
