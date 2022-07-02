// BASILISK
#include <bs_core.h>
#include <bs_shaders.h>
#include <bs_textures.h>
#include <bs_math.h>
#include <bs_models.h>
#include <bs_debug.h>

// STD
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <cglm/cglm.h>

bs_Batch batch;
bs_Tex2D tex;
bs_Model model;
bs_Shader shader;
bs_Framebuffer pixel;
bs_Camera camera;

float x_angle = 0.0;
float y_angle = 0.0;
float z_angle = 0.0;
float speed = 0.05;
versor v;

void render() {
    bs_selectBatch(&batch);

    bs_Mesh *mesh = &model.meshes[0];
    for(int i = 0; i < mesh->joint_count; i++) {
        bs_Joint *joint = &mesh->joints[i];

        bs_mat4 result = GLM_MAT4_IDENTITY_INIT;
        bs_mat4 bind;
        bs_mat4 local;
        memcpy(bind, mesh->joints[i].bind_matrix_inv, 16 * sizeof(float));
        memcpy(local, mesh->joints[i].mat, 16 * sizeof(float));
        glm_mat4_inv(bind, bind);

        glm_mat4_mul(result, local, result);
        glm_mat4_mul(result, bind, result);

        bs_vec3 translation = { result[3][0] * 30.0, result[3][1] * 30.0, result[3][2] * 30.0 + 100.0 };
        bs_vec3 p_translation = { result[3][0] * 30.0, result[3][1] * 30.0, result[3][2] * 30.0 };

        bs_RGBA color = { 255, 0, 0, 255 };

        if(i > 6 && i < 13) {
            color = (bs_RGBA){ 0, 127, 127, 255 };
        }

        if(strcmp("Root", joint->name) != 0) {
            // bs_pushRect(translation, (bs_vec2){ 4.0, 4.0 }, color);
        }
    }
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
    bs_init(1200, 900, "Test", bs_WND_DEFAULT);
    bs_setBackgroundColor((bs_fRGBA){ 60, 60, 80, 255 });
    loadTextures();

    camera.pos.x = -600.0;
    camera.pos.y = -300.0;
    camera.pos.z = 500.0;

    bs_createOrthographicProjection(&camera, 0, 1200, 0, 900);
    bs_setMatrixLookat(&camera, (bs_vec3){ -600.0, -300.0, -1.0 }, (bs_vec3){ 0.0, 1.0, 0.0 });

    shader = bs_loadShader("resources/bs_color_shader.vs", "resources/bs_color_shader.fs", 0);
    bs_createBatch(&batch, 200000);
    batch.shader = &shader;
    batch.camera = &camera;
    // batch.draw_mode = BS_LINES;
    bs_createFramebuffer(&pixel, 600, 450, pixelUpdate, NULL);

    for(int i = 0; i < model.meshes[0].joint_count; i++) {
        char uni_loc[256] = "boneMatrices[";
        char itoav[256];
        itoa(i, itoav, 10);
        strcat(uni_loc, itoav);
        strcat(uni_loc, "]\0");

        int loc = bs_getUniformLoc(batch.shader, uni_loc);
        bs_uniform_mat4(loc, model.meshes[0].joints[i].mat);
    }

    // bs_pushQuad(&quad);
    bs_pushBatch();

    bs_startRender(render);
    return 0;
}