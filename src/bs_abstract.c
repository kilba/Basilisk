/*
    This C file is an abstracted version of a lot of rendering features in
    in bs_core.c, it creates multiple batches for rendering simple shapes.
    Obviously this is not optimal for speed, but it is more than enough for
    programs with low to medium complexity.
*/

#include <bs_types.h>
#include <bs_math.h>
#include <bs_abstract.h>
#include <bs_core.h>
#include <bs_shaders.h>
#include <bs_textures.h>

enum {
    BATCH_TEX,
    BATCH_COL,
    BATCH_LINE,

    BATCH_COUNT
};

enum {
    SHADER_TEX,
    SHADER_COL,
    SHADER_NDC,

    SHADER_COUNT
};

bs_Shader shaders[SHADER_COUNT];
bs_Batch batches[BATCH_COUNT];
bs_Camera *camera;

/* --- W/ Camera --- */
void bs_drawTextureCam(bs_vec3 pos, bs_vec2 dim, bs_Texture *tex, bs_RGBA col, bs_Camera *cam) {
    bs_selectTexture(tex, 0);
    bs_selectBatch(batches + BATCH_TEX);
    batches[BATCH_TEX].camera = cam;
    
    bs_pushRect(pos, dim, col);

    bs_pushBatch();
    bs_renderBatch(0, bs_batchSize());
    bs_clearBatch();
}

void bs_drawFramebufPosCam(bs_Framebuf *framebuf, bs_vec2 pos, bs_vec2 dim, int attachment, bs_Camera *cam) {
    bs_drawTextureCam(bs_v3(pos.x, pos.y, 0.0), dim, framebuf->bufs + attachment, BS_WHITE, cam);
}

void bs_drawFramebufCam(bs_Framebuf *framebuf, int attachment, bs_Camera *cam) {
    bs_drawTextureCam(BS_V3_0, bs_v2(framebuf->dim.x, framebuf->dim.y), framebuf->bufs + attachment, BS_WHITE, cam);
}

void bs_drawRectCam(bs_vec3 pos, bs_vec2 dim, bs_RGBA col, bs_Camera *cam) {
    bs_selectBatch(batches + BATCH_COL);
    batches[BATCH_COL].camera = cam;
    
    bs_pushRect(pos, dim, col);

    bs_pushBatch();
    bs_renderBatch(0, bs_batchSize());
    bs_clearBatch();
}

void bs_drawTriangleCam(bs_vec3 pos0, bs_vec3 pos1, bs_vec3 pos2, bs_RGBA col, bs_Camera *cam) {
    bs_selectBatch(batches + BATCH_COL);
    batches[BATCH_COL].camera = cam;

    bs_pushTriangle(pos0, pos1, pos2, col);

    bs_pushBatch();
    bs_renderBatch(0, bs_batchSize());
    bs_clearBatch();
}

void bs_drawRectNDCCam(bs_vec3 pos, bs_vec2 dim, bs_RGBA col, bs_Camera *cam) {
    bs_selectBatch(batches + BATCH_COL);
    batches[BATCH_COL].camera = cam;
    batches[BATCH_COL].shader = shaders + SHADER_NDC;

    bs_pushRect(pos, dim, col);

    bs_pushBatch();
    bs_renderBatch(0, bs_batchSize());
    bs_clearBatch();
}

void bs_drawLineCam(bs_vec3 start, bs_vec3 end, bs_RGBA col, bs_Camera *cam) {
    bs_selectBatch(batches + BATCH_LINE);
    batches[BATCH_LINE].camera = cam;

    bs_pushLine(start, end, col);

    bs_pushBatch();
    bs_renderBatch(0, bs_batchSize());
    bs_clearBatch();
}

/* W/O Camera */
void bs_drawTexture(bs_vec3 pos, bs_vec2 dim, bs_Texture *tex, bs_RGBA col) {
    bs_drawTextureCam(pos, dim, tex,col, camera);
}

void bs_drawFramebufPos(bs_Framebuf *framebuf, bs_vec2 pos, bs_vec2 dim, int attachment) {
    bs_drawFramebufPosCam(framebuf, pos, dim, attachment, camera);
}

void bs_drawFramebuf(bs_Framebuf *framebuf, int attachment) {
    bs_drawFramebufCam(framebuf, attachment, camera);
}

void bs_drawRect(bs_vec3 pos, bs_vec2 dim, bs_RGBA col) {
    bs_drawRectCam(pos, dim, col, camera);
}

void bs_drawTriangle(bs_vec3 pos0, bs_vec3 pos1, bs_vec3 pos2, bs_RGBA col) {
    bs_drawTriangleCam(pos0, pos1, pos2, col, camera);
}

void bs_drawRectNDC(bs_vec3 pos, bs_vec2 dim, bs_RGBA col) {
    bs_drawRectNDCCam(pos, dim, col, camera);
}

void bs_drawLine(bs_vec3 start, bs_vec3 end, bs_RGBA col) {
    bs_drawLineCam(start, end, col, camera);
}

void bs_camAbstract(bs_Camera *cam) {
    camera = cam;
}

void bs_initAbstract() {
    char *vs_tex = \
	"#version 430\n"\
	"layout (location = 0) in vec3 bs_Pos;"\
	"layout (location = 1) in vec2 bs_TexCoord;"\
	"layout (location = 2) in vec4 bs_Color;"\
	"out vec2 ftex;"\
	"out vec4 fcol;"\
	"uniform mat4 bs_Proj;"\
	"uniform mat4 bs_View;"\
	"void main() {"\
	    "ftex = bs_TexCoord;"\
	    "fcol = bs_Color;"\
	    "gl_Position = bs_Proj * bs_View * vec4(bs_Pos, 1.0);"\
	"}";
    char *fs_tex = \
	"#version 430\n"\
	"in vec2 ftex;"\
	"in vec4 fcol;"\
	"in float fidx;"\
	"uniform sampler2D bs_Texture0;"\
	"out vec4 FragColor;"\
	"void main() {"\
	    "FragColor = texture(bs_Texture0, ftex) * fcol;"\
	"}";

    char *vs_col = \
	"#version 430\n"\
	"layout (location = 0) in vec3 bs_Pos;"\
	"layout (location = 1) in vec4 bs_Color;"\
	"out vec4 fcol;"\
	"uniform mat4 bs_Proj;"\
	"uniform mat4 bs_View;"\
	"void main() {"\
	    "fcol = bs_Color;"\
	    "gl_Position = bs_Proj * bs_View * vec4(bs_Pos, 1.0);"\
	"}";
    char *fs_col = \
	"#version 430\n"\
	"in vec4 fcol;"\
	"out vec4 FragColor;"\
	"void main() {"\
	    "FragColor = fcol;"\
	"}";
    char *vs_ndc = \
	"#version 430\n"\
	"layout (location = 0) in vec3 bs_Pos;"\
	"layout (location = 1) in vec4 bs_Color;"\
	"out vec4 fcol;"\
	"void main() {"\
	    "fcol = bs_Color;"\
	    "gl_Position = vec4(bs_Pos, 1.0);"\
	"}";


    bs_camAbstract(bs_defCamera());

    bs_shaderMem(shaders + SHADER_TEX, vs_tex, fs_tex, 0);
    bs_shaderMem(shaders + SHADER_COL, vs_col, fs_col, 0);
    bs_shaderMem(shaders + SHADER_NDC, vs_ndc, fs_col, 0);
    bs_batch(batches + BATCH_TEX, shaders + SHADER_TEX);
    bs_batch(batches + BATCH_COL, shaders + SHADER_COL);
    bs_batch(batches + BATCH_LINE, shaders + SHADER_COL);
    batches[BATCH_LINE].draw_mode = BS_LINES;
}
