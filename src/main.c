// BASILISK
#include <bs_core.h>
#include <bs_shaders.h>
#include <bs_textures.h>
#include <bs_models.h>
#include <bs_debug.h>

// STD
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <cglm/cglm.h>

bs_Batch batch;
bs_Quad quad;
bs_Tex2D *tex;
bs_Model model;
bs_Shader shader;
bs_Framebuffer pixel;

float x_angle = 0.0;
float y_angle = 0.0;
float z_angle = 0.0;
float speed = 0.05;
versor v;

void render() {

    // glm_quat(v, x_angle, 0.0, 1.0, 0.0);

    if(bs_isKeyDown(BS_KEY_W)) {
        x_angle += speed;
        glm_quat(v, x_angle, 1.0, 0.0, 0.0);
    }

    if(bs_isKeyDown(BS_KEY_S)) {
        x_angle -= speed;
        glm_quat(v, x_angle, 1.0, 0.0, 0.0);
    }

    if(bs_isKeyDown(BS_KEY_D)) {
        y_angle += speed;
        glm_quat(v, y_angle, 0.0, 1.0, 0.0);
    }

    if(bs_isKeyDown(BS_KEY_A)) {
        y_angle -= speed;
        glm_quat(v, y_angle, 0.0, 1.0, 0.0);
    }

    if(bs_isKeyDown(BS_KEY_Q)) {
        z_angle -= speed;
        glm_quat(v, z_angle, 0.0, 0.0, 1.0);
    }

    if(bs_isKeyDown(BS_KEY_E)) {
        z_angle += speed;
        glm_quat(v, z_angle, 0.0, 0.0, 1.0);
    }

    bs_selectBatch(&batch);

    model.meshes[0].rot = (bs_vec4){ v[0], v[1], v[2], v[3] };
    bs_pushModel(&model);

    bs_pushBatch();
    bs_renderBatch(0, bs_getBatchSize(&batch));
    bs_clearBatch();
}

void loadTextures() {
    bs_loadModel("resources/models/untitled.gltf", "resources/models/textures", &model);
}

void pixelUpdate() {
}

int main() {
    bs_init(1200, 800, "Test", bs_WND_DEFAULT);
    bs_setBackgroundColor((bs_fRGBA){ 80, 100, 120, 255 });
    loadTextures();

    shader = bs_loadShader("resources/bs_color_shader2.vs", "resources/bs_color_shader2.fs", 0);
    bs_createBatch(&batch, 20000, BS_POSITION_COLOR);
    // batch.shader = &shader;

    // bs_createFramebuffer(&pixel, 1200, 800, pixelUpdate, NULL);

    quad.pos.x = 0.0;
    quad.pos.y = 0.0;
    quad.pos.z = 0.0;

    quad.dim.x = 100.0;
    quad.dim.y = 100.0;

    quad.col.r = 255;
    quad.col.g = 255;
    quad.col.b = 255;
    quad.col.a = 255;

    quad.tex = tex;

    // bs_pushQuad(&quad);
    bs_pushBatch();

    bs_startRender(render);
    return 0;
}