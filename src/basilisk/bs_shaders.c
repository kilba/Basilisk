// GL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
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

int loaded_shader_count = 0;
const char *std_uniforms[] = { "bs_Proj", "bs_View", "bs_Time" };

// INITIALIZATION
// Gets all default uniform locations
void bs_setDefaultUniforms(bs_Shader *shader, char *shader_code){
    for (int i = 0; i < UNIFORM_TYPE_COUNT; i++) {
        // Check if the shader contains the uniform
        if(strstr(shader_code, std_uniforms[i])){
            bs_Uniform *uniform = &shader->uniforms[i];

            // If uniform already has been set
            if(uniform->is_valid)
                continue;

            int uniform_loc = glGetUniformLocation(shader->id, std_uniforms[i]);
            // If uniform is unused or non existent
            if(uniform_loc == -1)
                continue;

            uniform->is_valid = true;
            uniform->loc = uniform_loc;
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
    for(int i = 0; i < UNIFORM_TYPE_COUNT; i++) {
        shader->uniforms[i].is_valid = false;
    }

    bs_setDefaultUniforms(shader, (char *)vs_code);
    bs_setDefaultUniforms(shader, (char *)fs_code);

    if(gs_code != 0) {
        bs_setDefaultUniforms(shader, (char *)gs_code);
    }
}

void bs_loadMemShader(char *vs_code, char *fs_code, char *gs_code, bs_Shader *shader) {
    if(vs_code == NULL || fs_code == NULL) {
        shader->id = -1;
        return;
    }

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

    bs_Camera *cam = bs_getStdCamera();
    bs_setDefaultUniformLocations(shader, vs_code, fs_code, gs_code);
    bs_setViewMatrixUniform(shader, cam);
    bs_setProjMatrixUniform(shader, cam);

    shader->index = loaded_shader_count;
    loaded_shader_count++;

    return;
}

void bs_loadShader(char *vs_path, char *fs_path, char *gs_path, bs_Shader *shader) {
    int vs_err_code;
    int fs_err_code;
    int gs_err_code;

    // Load shader source code into memory
    // TODO: Free
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
}

/* COMPUTE SHADERS */
void bs_loadMemComputeShader(char *cs_code, bs_ComputeShader *compute_shader, bs_Texture *tex) {
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

void bs_loadComputeShader(char *cs_path, bs_ComputeShader *compute_shader, bs_Texture *tex) {
    int cs_err_code;
    int len;

    char *cscode = bs_readFileToString(cs_path, &len, &cs_err_code);
    bs_loadMemComputeShader(cscode, compute_shader, tex);
}

void bs_getUniformLoc(bs_Shader *shader, char *name, int *result) {
    *result = glGetUniformLocation(shader->id, name);
}

void bs_setShaderAtlas(bs_Shader *shader, bs_Atlas *atlas, char *uniform_name) {
    bs_switchShader(shader);
    int loc = glGetUniformLocation(shader->id, uniform_name);
    glUniform1i(loc, atlas->tex.id);
}

// SETTING DEFAULT UNIFORMS
void bs_setTimeUniform(bs_Shader *shader, float time) {
    bs_Uniform *uniform = &shader->uniforms[UNIFORM_TIME];

    // Don't set the uniform if it doesn't exist
    if(!uniform->is_valid)
        return;

    bs_uniform_float(uniform->loc, time);
}

void bs_setViewMatrixUniform(bs_Shader *shader, void *cam) {
    bs_switchShader(shader);
    bs_Uniform *uniform = &shader->uniforms[UNIFORM_VIEW];

    if(!uniform->is_valid)
        return;

    bs_uniform_mat4(uniform->loc, ((bs_Camera*)cam)->view);
}

void bs_setProjMatrixUniform(bs_Shader *shader, void *cam) {
    bs_switchShader(shader);
    bs_Uniform *uniform = &shader->uniforms[UNIFORM_PROJ];

    if(!uniform->is_valid)
        return;

    bs_uniform_mat4(uniform->loc, ((bs_Camera*)cam)->proj);
}

// SHADER ABSTRACTION LAYER
void bs_switchShader(bs_Shader *shader) {
    glUseProgram(shader->id);
}

void bs_switchShaderCompute(bs_ComputeShader *compute_shader) {
    glUseProgram(compute_shader->id);
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

// VECTORS
// TODO: bvecn, ivecn, uvecn, vecn, dvecn

// TODO: Arrays