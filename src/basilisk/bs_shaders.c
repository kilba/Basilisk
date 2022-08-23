// GL
#include <glad/glad.h>
#include <cglm/cglm.h>

// Basilisk
#include <bs_shaders.h>
#include <bs_core.h>
#include <bs_debug.h>
#include <bs_file_mgmt.h>
#include <bs_textures.h>

// STD
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

// INITIALIZATION
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

        bs_getUniformLoc(shader->id, uni_texture, &loc);
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

   const char s[2] = "";
   char *token;
   
   /* get the first token */
   token = strtok(vs_code, s);
   
   /* walk through other tokens */
   while( token != NULL ) {
      token = strtok(NULL, s);
   }

    int attrib_count = sizeof(def_attribs) / sizeof(char*);
    for(int i = 0; i < attrib_count; i++) {
        if(strstr(vs_code, def_attribs[i])) {
            shader->attribs |= values[i];
            shader->attrib_count++;
        }
    }
}

void bs_shaderErrorCheck(GLuint *shader, int shadertype) {
    GLint isCompiled = 0;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
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

void bs_loadShaderCode(GLuint *shader, const GLchar *shader_code, int type) {
    *shader = glCreateShader(type);
    glShaderSource (*shader, 1, &shader_code, NULL);
    glCompileShader(*shader);

    bs_shaderErrorCheck(shader, type);
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

    bs_loadShaderCode(&shader->vs_id, vs_code, GL_VERTEX_SHADER);
    bs_loadShaderCode(&shader->fs_id, fs_code, GL_FRAGMENT_SHADER);

    glAttachShader(shader->id, shader->vs_id);
    glAttachShader(shader->id, shader->fs_id);

    // Geometry shader is not mandatory
    if(gs_code != 0) {
        bs_loadShaderCode(&shader->gs_id, gs_code, GL_GEOMETRY_SHADER);
        glAttachShader(shader->id, shader->gs_id);
    }

    glLinkProgram(shader->id);
    glUseProgram(shader->id);

    // bs_Camera *cam = bs_getStdCamera();
    bs_setDefaultUniformLocations(shader, vs_code, fs_code, gs_code);
    // bs_setViewMatrixUniform(shader, cam);
    // bs_setProjMatrixUniform(shader, cam);

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

    bs_loadShaderCode(&compute_shader->cs_id, cs_code, GL_COMPUTE_SHADER);

    compute_shader->id = glCreateProgram();
    glAttachShader(compute_shader->id, compute_shader->cs_id);
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

void bs_getUniformLoc(int id, char *name, int *result) {
    *result = glGetUniformLocation(id, name);
}

void bs_setShaderAtlas(bs_Shader *shader, bs_Atlas *atlas, char *uniform_name) {
    bs_switchShader(shader->id);
    int loc = glGetUniformLocation(shader->id, uniform_name);
    glUniform1i(loc, atlas->tex.id);
}

// SETTING DEFAULT UNIFORMS
void bs_setViewMatrixUniform(bs_Shader *shader, void *cam) {
    bs_Uniform *uniform = &shader->uniforms[UNIFORM_VIEW];

    if(!uniform->is_valid)
        return;

    bs_switchShader(shader->id);
    bs_uniform_mat4(uniform->loc, ((bs_Camera*)cam)->view);
}

void bs_setProjMatrixUniform(bs_Shader *shader, void *cam) {
    bs_Uniform *uniform = &shader->uniforms[UNIFORM_PROJ];

    if(!uniform->is_valid)
        return;

    bs_switchShader(shader->id);
    bs_uniform_mat4(uniform->loc, ((bs_Camera*)cam)->proj);
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

void bs_uniform_vec3(int loc, bs_vec3 vec) {
    glUniform3f(loc, vec[0], vec[1], vec[2]);
}

// TODO: Arrays