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

bs_Shader test_shader;
bs_Shader test_shader2;
bs_Shader fbo_shader;

bs_Batch test_batch;
bs_Batch test_batch2;
bs_Atlas *atlas;
bs_Tex2D *anim;

bs_Framebuffer fbo_p;
bs_Camera cam;

void render() {
    bs_selectAtlas(atlas);
    bs_selectBatch(&test_batch);

    bs_renderBatch(0, bs_getBatchSize(&test_batch));
    bs_debugUpdate();

cam.pos.x-=0.1;
}

void loadTextures() {
    atlas = bs_createTextureAtlas(256, 256, 100);
    anim = bs_loadTexture("resources/idle.png", 1, atlas);

    bs_pushAtlas(atlas);
    // bs_saveAtlasToFile(atlas, "atlas1.png");
    bs_freeAtlasData(atlas);

    bs_selectTexture(anim+0);
}

void pixel_render() {
    bs_selectBatch(&test_batch2);
    bs_renderBatch(0, bs_getBatchSize(&test_batch2));
}

int main() {
    bs_init(1200, 800, "Test", bs_WND_DEFAULT);

    cam.pos.x = 0.0;
    cam.pos.y = 0.0;
    cam.pos.z = 200.0;
    bs_setMatrixLookat(&cam, (bs_vec3){ 0.0, 0.0, 2.0 }, (bs_vec3){ 0.0, 1.0, 0.0 });
    bs_setPerspectiveProjection(&cam, 90.0, 1200.0 / 800.0, 0.1, 5000.0);

    loadTextures();
    fbo_shader = bs_loadShader("resources/fbo_shader.vs", "resources/fbo_shader.fs", 0);

    bs_createFramebuffer(&fbo_p, 1200, 800, pixel_render, &fbo_shader);

    test_shader = bs_loadShader("resources/bs_color_shader.vs", "resources/bs_color_shader.fs", 0);
    test_shader2 = bs_loadShader("resources/bs_color_shader2.vs", "resources/bs_color_shader2.fs", 0);
    bs_setBackgroundColor((bs_fRGBA){ 0, 30, 40, 0 });

    bs_createBatch(&test_batch, 4 * 128 * 128, BS_STATIC_BATCH, BS_POSITION_COLOR);
    test_batch.camera = &cam;

    for(int y = 0; y < 128; y++) {
        for(int x = 0; x < 128; x++) {
            bs_pushQuad((bs_vec2){ 10.0 + x * 2.0, 200.0 + y * 2.0 }, (bs_vec2){ 100.0, 100.0 }, (bs_RGBA){ x * 2, y * 2, x + y, 255 });
        }
    }

    bs_pushQuad((bs_vec2){ 1.0, 1.0 }, (bs_vec2){ 100.0, 100.0 }, (bs_RGBA){ 120, 20, 80, 255 });
    bs_pushTriangle((bs_vec2){ 400.0, 200.0 }, (bs_vec2){ 500.0, 200.0 }, (bs_vec2){ 500.0, 300.0 }, (bs_RGBA){ 120, 20, 80, 255 });

    bs_pushBatch();
    bs_freeBatchData();

    bs_Model model = bs_loadModel();

    bs_createBatch(&test_batch2, model.vertex_count * 3, BS_STATIC_BATCH, BS_POSITION_COLOR);
    test_batch2.shader = &test_shader2;
    // test_batch2.camera = &cam;

    printf("MYCAM\n");
    for(int y = 0; y < 4; y++) {
        for(int x = 0; x < 4; x++) {
            printf("%f\n", cam.proj[x][y]);
            printf("%f\n", cam.view[x][y]);
            printf("\n");
        }
    }


    for(int i = 0; i < model.vertex_count; i++) {
        bs_pushVertex(model.vertices[i].x, model.vertices[i].y, model.vertices[i].z, 0.0, 0.0, 0.0, 0.0, 0.0, (bs_RGBA){ 255, 0, 0, 255 });
    }

    test_batch2.indices = malloc(model.index_count * sizeof(int));
    memcpy(test_batch2.indices, model.indices, model.index_count * sizeof(int));
    test_batch2.index_draw_count = model.index_count;
    bs_pushBatch();
    bs_freeBatchData();

    bs_debugStart();

    bs_startRender(render);
    return 0;
}