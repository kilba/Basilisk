#include "bs_types.h"
#include <bs_core.h>
#include <bs_wnd.h>
#include <bs_textures.h>
#include <bs_shaders.h>

#include <stdio.h>
#include <stdarg.h>

#include <lodepng.h>
#include <cglm/cglm.h>
#include <glad/glad.h>

char *vs_code = \
    "#version 430\n" \
    "layout (location = 0) in vec3 bs_Pos;" \

    "uniform mat4 bs_Proj;" \
    "uniform mat4 bs_View;" \
    "uniform mat4 model;" \

    "void main() {" \
	"gl_Position = bs_Proj * bs_View * model * vec4(bs_Pos, 1.0);" \
    "}";

char *fs_code = \
    "#version 430\n" \
    "layout (location = 0) out vec4 FragColor;" \
    "void main() {" \
	"FragColor = vec4(1.0);" \
    "}";

/* Primitive Selection Structure */
struct {
    int model_loc;
    int data; /* Hex */

    bs_Batch batch;
    bs_Shader shader;
    bs_Tex2D buf;
    bs_Framebuffer framebuf;
} selection_data;

void bs_print(const int info_type, char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
}

bool bsdbg_hilightMeshOnMouseOver(bs_Mesh *mesh, bs_mat4 model, bs_Camera *cam) {
    selection_data.batch.camera = cam;

    glDrawBuffer(GL_BACK);
    bs_selectBatch(&selection_data.batch);

    bs_uniform_mat4(selection_data.model_loc, model);

    bs_pushMesh(mesh);
    bs_pushBatch();

    bs_renderBatch(0, bs_batchSize());
    bs_clearBatch();
    glReadBuffer(GL_BACK);


    bs_ivec2 coord = bs_cursorPosWndInv();
    glReadPixels(coord.x, coord.y, 1, 1, BS_CHANNEL_RGBA, BS_UBYTE, &selection_data.data);

    glClear(GL_COLOR_BUFFER_BIT);
    return (selection_data.data == 0xFFFFFFFF);
}

void bsdbg_tick() {
}

void bsdbg_init() {
    bs_ivec2 res = bs_resolution();

    bs_textureRGBA(&selection_data.buf, res);
    bs_loadMemShader(vs_code, fs_code, 0, &selection_data.shader);
    selection_data.model_loc = bs_uniformLoc(selection_data.shader.id, "model");

    bs_framebuffer(&selection_data.framebuf, res);
    bs_attachColorbuffer(&selection_data.buf, 0);

    bs_batch(&selection_data.batch, &selection_data.shader, 10000);
}
