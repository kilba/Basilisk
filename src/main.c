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

bs_Shader test_shader;
bs_Shader test_shader2;
bs_Shader fbo_shader;

bs_Batch test_batch;
bs_Batch test_batch2;
bs_Atlas *atlas;
bs_Tex2D *anim;

bs_Framebuffer fbo_p;
bs_Camera cam;

bs_Quad quads[100];
    bs_Model model;
void render() {
    bs_selectAtlas(atlas);

    bs_selectBatch(&test_batch);
    bs_renderBatch(0, bs_getBatchSize(&test_batch));

    bs_debugUpdate();

    bs_selectBatch(&test_batch2);

    quads[0].pos.x -= 0.1;
    quads[0].pos.y = sin(quads[0].pos.x) - cos(quads[0].pos.x) * 2.0;
    bs_pushMesh(&model.meshes[0]);
    bs_pushMesh(&model.meshes[1]);
    bs_pushQuad(&quads[0]);
    bs_pushMesh(&model.meshes[2]);
    // bs_pushTriangle((bs_vec2){0.0, 0.0}, (bs_vec2){0.0, 100.0}, (bs_vec2){100.0, 100.0}, (bs_RGBA){ 255, 0, 0, 255 });

    bs_pushBatch();

    bs_renderBatch(0, bs_getBatchSize(&test_batch2));
    bs_clearBatch();
}

void loadTextures() {
    atlas = bs_createTextureAtlas(256, 256, 100);
    anim = bs_loadTexture("resources/idle.png", 1, atlas);

    bs_pushAtlas(atlas);
    // bs_saveAtlasToFile(atlas, "atlas1.png");
    bs_freeAtlasData(atlas);

    bs_selectTexture(anim+0);

}

int main() {
    bs_init(1200, 800, "Test", bs_WND_DEFAULT);

    cam.pos.x = 0.0;
    cam.pos.y = 0.0;
    cam.pos.z = 200.0;
    // bs_setMatrixLookat(&cam, (bs_vec3){ 0.0, 0.0, 2.0 }, (bs_vec3){ 0.0, 1.0, 0.0 });
    // bs_setPerspectiveProjection(&cam, 90.0, 1200.0 / 800.0, 0.1, 5000.0);
    bs_createOrthographicProjection(&cam, 0, 1200, 0, 800);

    loadTextures();
    fbo_shader = bs_loadShader("resources/fbo_shader.vs", "resources/fbo_shader.fs", 0);
    test_shader = bs_loadShader("resources/bs_color_shader.vs", "resources/bs_color_shader.fs", 0);
    test_shader2 = bs_loadShader("resources/bs_color_shader2.vs", "resources/bs_color_shader2.fs", 0);
    bs_setBackgroundColor((bs_fRGBA){ 0, 30, 40, 0 });

    bs_loadModel("resources/untitled.gltf", &model);

    int index_count = 6 * 4;
    index_count += model.meshes[0].index_count;
    bs_createBatch(&test_batch, index_count, BS_POSITION_COLOR);
    test_batch.camera = &cam;

    quads[0].tex = anim;
    quads[0].pos = (bs_vec3){ 100.0, 100.0, 0.0 };
    quads[0].dim = (bs_vec2){ quads[0].tex->w, quads[0].tex->h };
    quads[0].col = (bs_RGBA){ 120, 20, 80, 100 };
    bs_pushQuad(&quads[0]);
    quads[0].pos.x += 50.0;
    bs_pushQuad(&quads[0]);
    quads[0].pos.x += 50.0;
    bs_pushQuad(&quads[0]);

    bs_pushBatch();
    bs_freeBatchData();

    bs_createBatch(&test_batch2, 20000, BS_POSITION_COLOR);
    test_batch2.shader = &test_shader2;

    bs_debugStart();

    bs_startRender(render);
    return 0;
}