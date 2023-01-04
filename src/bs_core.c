// GL
#include <glad/glad.h>
#include <cglm/cglm.h>

// Basilisk
#include <bs_shaders.h>
#include <bs_textures.h>
#include <bs_core.h>
#include <bs_math.h>
#include <bs_wnd.h>

#include <bs_debug.h>

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <lodepng.h>
#include <assert.h>

#ifdef _WIN32
    #include <windows.h>
    #include <objidl.h>
#endif

#include <time.h>

bs_Camera def_camera;
bs_Batch *curr_batch;

bs_Framebuf *curr_framebuf = NULL;
bs_UniformBuffer global_unifs;

int bs_checkError() {
    GLenum err = glGetError();
    return err; 
}

/* --- MATRICES / CAMERAS --- */
bs_Camera *bs_defCamera() {
    return &def_camera;
}

void bs_persp(bs_Camera *cam, float aspect, float fovy, float nearZ, float farZ) {
    glm_perspective(glm_rad(fovy), aspect, nearZ, farZ, cam->proj);
}

void bs_ortho(bs_Camera *cam, int left, int right, int bottom, int top, float nearZ, float farZ) {
    glm_ortho(left, right, bottom, top, nearZ, farZ, cam->proj);

    int x_res = bs_sign(left - right);
    int y_res = bs_sign(top - bottom);
}

void bs_lookat(bs_Camera *cam, bs_vec3 eye, bs_vec3 center, bs_vec3 up) {
    glm_lookat((vec3){ eye.x, eye.y, eye.z }, (vec3){ center.x, center.y, center.z }, (vec3){ up.x, up.y, up.z }, cam->view);
}

void bs_look(bs_Camera *cam, bs_vec3 eye, bs_vec3 dir, bs_vec3 up) {
    glm_look((vec3){ eye.x, eye.y, eye.z }, (vec3){ dir.x, dir.y, dir.z }, (vec3){ up.x, up.y, up.z }, cam->view);
}

/* --- BATCHED RENDERING --- */
void bs_selectBatch(bs_Batch *batch) {
    curr_batch = batch;

    glBindVertexArray(batch->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, batch->VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->EBO);

    bs_switchShader(curr_batch->shader->id);
}

void bs_batchResizeCheck(int index_count, int vertex_count) {
    bs_Batch *batch = curr_batch;
    if((batch->index_draw_count + index_count) < batch->allocated_index_count)
	if((batch->vertex_draw_count + vertex_count) < batch->allocated_vertex_count)
	    return;

    int new_index_count = batch->index_draw_count + BS_BATCH_INCR_BY + index_count;
    int new_vertex_count = batch->vertex_draw_count + BS_BATCH_INCR_BY + vertex_count;

    bs_batchBufferSize(new_index_count, new_vertex_count);
}

void bs_pushIndex(int idx) {
    curr_batch->indices[curr_batch->index_draw_count++] = curr_batch->vertex_draw_count + idx;
}

void bs_pushIndices(int *idxs, int num_elems) {
    for(int i = 0; i < num_elems; i++)
	bs_pushIndex(idxs[i]);
}

void bs_pushIndexVa(int num_elems, ...) {
    va_list ptr;
    va_start(ptr, num_elems);

    for(int i = 0; i < num_elems; i++)
	bs_pushIndex(va_arg(ptr, int));

    va_end(ptr);
}

void bs_pushAttrib(uint8_t **data_ptr, void *data, uint8_t size) {
    memcpy(*data_ptr, data, size);
    *data_ptr += size;
}

void bs_pushVertex(
    bs_vec3  pos,
    bs_vec2  tex,
    bs_vec3  nor,
    bs_RGBA  col,
    bs_ivec4 bid,
    bs_vec4  wei,
    bs_vec4  v4_,
    bs_ivec4 v4i
) {
    bs_Batch *batch = curr_batch;

    uint8_t *data_ptr = (uint8_t *)batch->vertices + batch->vertex_draw_count * batch->attrib_size_bytes;
    uint8_t *sizes = batch->shader->attrib_sizes;
    
    bs_pushAttrib(&data_ptr, &pos, sizes[0]);
    bs_pushAttrib(&data_ptr, &tex, sizes[1]);
    bs_pushAttrib(&data_ptr, &col, sizes[2]);
    bs_pushAttrib(&data_ptr, &nor, sizes[3]);
    bs_pushAttrib(&data_ptr, &bid, sizes[4]);
    bs_pushAttrib(&data_ptr, &wei, sizes[5]);
    bs_pushAttrib(&data_ptr, &v4_, sizes[6]);
    bs_pushAttrib(&data_ptr, &v4i, sizes[7]);
    
    curr_batch->vertex_draw_count++;
} 

int bs_pushQuad(bs_vec3 p0, bs_vec3 p1, bs_vec3 p2, bs_vec3 p3, bs_RGBA col) {
    bs_batchResizeCheck(6, 4);

    bs_pushIndexVa(6, 0, 1, 2, 2, 1, 3);

    bs_pushVertex(p0, (bs_vec2){ 0.0, 0.0 }, BS_V3_0, col, BS_IV4_0, BS_V4_0, BS_V4_0, BS_IV4_0); // Bottom Left
    bs_pushVertex(p1, (bs_vec2){ 1.0, 0.0 }, BS_V3_0, col, BS_IV4_0, BS_V4_0, BS_V4_0, BS_IV4_0); // Bottom right
    bs_pushVertex(p2, (bs_vec2){ 0.0, 1.0 }, BS_V3_0, col, BS_IV4_0, BS_V4_0, BS_V4_0, BS_IV4_0); // Top Left
    bs_pushVertex(p3, (bs_vec2){ 1.0, 1.0 }, BS_V3_0, col, BS_IV4_0, BS_V4_0, BS_V4_0, BS_IV4_0); // Top Right

    return curr_batch->index_draw_count;
}

int bs_pushRectCoord(bs_vec3 pos, bs_vec2 dim, bs_vec2 tex0, bs_vec2 tex1, bs_RGBA col) {
    bs_batchResizeCheck(6, 4);

    dim.x += pos.x;
    dim.y += pos.y;

    bs_pushIndexVa(6, 0, 1, 2, 2, 1, 3);

    bs_pushVertex((bs_vec3){ pos.x, pos.y, pos.z }, (bs_vec2){ tex0.x, tex1.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0, BS_V4_0, BS_IV4_0);
    bs_pushVertex((bs_vec3){ dim.x, pos.y, pos.z }, (bs_vec2){ tex1.x, tex1.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0, BS_V4_0, BS_IV4_0);
    bs_pushVertex((bs_vec3){ pos.x, dim.y, pos.z }, (bs_vec2){ tex0.x, tex0.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0, BS_V4_0, BS_IV4_0);
    bs_pushVertex((bs_vec3){ dim.x, dim.y, pos.z }, (bs_vec2){ tex1.x, tex0.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0, BS_V4_0, BS_IV4_0);

    return curr_batch->index_draw_count;
}

int bs_pushRectRotated(bs_vec3 pos, bs_vec2 dim, float angle, bs_RGBA col) {
    bs_batchResizeCheck(6, 4);
    bs_Texture *tex = bs_selectedTexture();

    dim.x += pos.x;
    dim.y += pos.y;

    bs_pushIndexVa(6, 0, 1, 2, 2, 1, 3);

    bs_vec3 p0, p1, p2, p3;
    p0 = BS_V2_Z(bs_v2rot(BS_V2(pos.x, pos.y), BS_V2(pos.x, pos.y), angle), pos.z);
    p1 = BS_V2_Z(bs_v2rot(BS_V2(dim.x, pos.y), BS_V2(pos.x, pos.y), angle), pos.z);
    p2 = BS_V2_Z(bs_v2rot(BS_V2(pos.x, dim.y), BS_V2(pos.x, pos.y), angle), pos.z);
    p3 = BS_V2_Z(bs_v2rot(BS_V2(dim.x, dim.y), BS_V2(pos.x, pos.y), angle), pos.z);

    bs_vec2 tex0, tex1;
    tex0.x = tex->texw * (float)tex->frame.x;
    tex1.y = tex->texh * (float)tex->frame.y;
    tex1.x = tex0.x + tex->texw;
    tex0.y = tex0.y + tex->texh;

    bs_pushVertex(p0, (bs_vec2){ tex0.x, tex1.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0, BS_V4_0, BS_IV4_0);
    bs_pushVertex(p1, (bs_vec2){ tex1.x, tex1.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0, BS_V4_0, BS_IV4_0);
    bs_pushVertex(p2, (bs_vec2){ tex0.x, tex0.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0, BS_V4_0, BS_IV4_0);
    bs_pushVertex(p3, (bs_vec2){ tex1.x, tex0.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0, BS_V4_0, BS_IV4_0);

    return curr_batch->index_draw_count;
}

int bs_pushRectFlipped(bs_vec3 pos, bs_vec2 dim, bs_RGBA col) {
    bs_Texture *tex = bs_selectedTexture();

    bs_vec2 tex0, tex1;
    tex0.x = tex->texw * (float)tex->frame.x;
    tex0.y = tex->texh * (float)tex->frame.y;
    tex1.x = tex0.x + tex->texw;
    tex1.y = tex0.y + tex->texh;

    return bs_pushRectCoord(pos, dim, tex0, tex1, col);
}

int bs_pushRect(bs_vec3 pos, bs_vec2 dim, bs_RGBA col) {
    bs_batchResizeCheck(6, 4);
    bs_Texture *tex = bs_selectedTexture();

    bs_vec2 tex0, tex1;
    tex0.x = tex->texw * (float)tex->frame.x;
    tex1.y = tex->texh * (float)tex->frame.y;
    tex1.x = tex0.x + tex->texw;
    tex0.y = tex0.y + tex->texh;

    return bs_pushRectCoord(pos, dim, tex0, tex1, col);
}

int bs_pushTriangle(bs_vec3 pos1, bs_vec3 pos2, bs_vec3 pos3, bs_RGBA color) {
    bs_batchResizeCheck(3, 3);
    
    bs_pushIndexVa(3, 0, 1, 2),

    bs_pushVertex(pos1, (bs_vec2){ 0.0, 0.0 }, BS_V3_0, color, BS_IV4_0, BS_V4_0, BS_V4_0, BS_IV4_0);
    bs_pushVertex(pos2, (bs_vec2){ 1.0, 0.0 }, BS_V3_0, color, BS_IV4_0, BS_V4_0, BS_V4_0, BS_IV4_0);
    bs_pushVertex(pos3, (bs_vec2){ 0.0, 1.0 }, BS_V3_0, color, BS_IV4_0, BS_V4_0, BS_V4_0, BS_IV4_0);

    return curr_batch->index_draw_count += 3;
}

int bs_pushLine(bs_vec3 start, bs_vec3 end, bs_RGBA color) {
    return bs_pushTriangle(start, end, end, color);
}

/* --- Rendering models with attributes --- */
int bs_pushPrimA(bs_Prim *prim, bs_vec4 attributes) {
    bs_batchResizeCheck(prim->index_count, prim->vertex_count);

    bs_pushIndices(prim->indices, prim->index_count);

    float *vertex = prim->vertices;
    for(int i = 0; i < prim->vertex_count; i++, vertex += prim->vertex_size) {
        bs_pushVertex(
            *(bs_vec3  *)(vertex + 0),
	    *(bs_vec2  *)(vertex + prim->offset_tex),
	    *(bs_vec3  *)(vertex + prim->offset_nor),
            prim->material.col, 
	    *(bs_ivec4 *)(vertex + prim->offset_bid),
	    *(bs_vec4  *)(vertex + prim->offset_wei),
            attributes,
	    BS_IV4_0
        );
    }

    return curr_batch->index_draw_count;
}

int bs_pushMeshA(bs_Mesh *mesh, bs_vec4 attributes) {
    int ret = 0;
    for(int i = 0; i < mesh->prim_count; i++) {
        bs_Prim *prim = &mesh->prims[i];
        ret += bs_pushPrimA(prim, attributes);
    }
    return ret;
}

int bs_pushModelA(bs_Model *model, bs_vec4 attributes) {
    int ret = 0;
    for(int i = 0; i < model->mesh_count; i++) {
        ret += bs_pushMeshA(&model->meshes[i], attributes);
    }
    return ret;
}

int bs_pushPrim(bs_Prim *prim) {
    return bs_pushPrimA(prim, BS_V4_0);
}

int bs_pushMesh(bs_Mesh *mesh) {
    return bs_pushMeshA(mesh, BS_V4_0);
}

int bs_pushModel(bs_Model *model) {
    return bs_pushModelA(model, BS_V4_0);
}

void bs_batchBufferSize(int index_count, int vertex_count) {
    bs_Batch *batch = curr_batch;

    batch->allocated_index_count = index_count;
    batch->allocated_vertex_count = vertex_count;

    batch->vertices = realloc(batch->vertices, vertex_count * batch->attrib_size_bytes);
    batch->indices  = realloc(batch->indices , index_count * sizeof(int));

    glBufferData(GL_ARRAY_BUFFER, vertex_count * batch->attrib_size_bytes, batch->vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * index_count, batch->indices, GL_STATIC_DRAW);
}

void bs_batch(bs_Batch *batch, bs_Shader *shader) {
    // Default values
    memset(batch, 0, sizeof(bs_Batch));
    batch->draw_mode = BS_TRIANGLES;
    batch->camera = &def_camera;
    batch->shader = shader;

    // Create buffer/array objects
    glGenVertexArrays(1, &batch->VAO);
    glGenBuffers(1, &batch->VBO);
    glGenBuffers(1, &batch->EBO);

    bs_selectBatch(batch);

    // Attribute setup
    struct Attrib_Data {
        int type;
        int count;
        int size;
        bool normalized;
    } attrib_data[] = {
        { BS_FLOAT, 3, sizeof(bs_vec3) , false }, /* Position */
        { BS_FLOAT, 2, sizeof(bs_vec2) , false }, /* Tex Coord */
        { BS_UBYTE, 4, sizeof(bs_RGBA) , true  }, /* Color */
        { BS_FLOAT, 3, sizeof(bs_vec3) , false }, /* Normal */
        { BS_INT  , 4, sizeof(bs_ivec4), false }, /* Bone Ids */
        { BS_FLOAT, 4, sizeof(bs_vec4) , false }, /* Weights */
        { BS_FLOAT, 4, sizeof(bs_vec4) , false }, /* Vec4 Attrib */
    };
    int total_attrib_count = sizeof(attrib_data) / sizeof(struct Attrib_Data);

    // Calculate attrib sizes
    int i = 0, j = 1;
    for(; i < total_attrib_count; i++, j *= 2) {
        struct Attrib_Data *data = &attrib_data[i];

        if((batch->shader->attribs & j) == j)
            batch->attrib_size_bytes += data->size;
    }

    // Add attributes
    i = 0; j = 1;
    for(; i < total_attrib_count; i++, j *= 2) {
        struct Attrib_Data *data = &attrib_data[i];

        if((batch->shader->attribs & j) == j)
            bs_attrib(data->type, data->count, data->size, data->normalized);
    }
}

void bs_batchRawData(void *vertex_data, void *index_data, int vertex_size, int index_size) {
    glBufferData(GL_ARRAY_BUFFER, vertex_size, vertex_data, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_size, index_data, GL_STATIC_DRAW);
}

void bs_attribI(const int type, unsigned int amount, size_t size_per_type) {
    bs_Batch *batch = curr_batch;

    glEnableVertexAttribArray(batch->attrib_count);
    glVertexAttribIPointer(batch->attrib_count++, amount, type, batch->attrib_size_bytes, (void*)batch->attrib_offset);

    batch->attrib_offset += size_per_type;
}

void bs_attrib(const int type, unsigned int amount, size_t size_per_type, bool normalized) {
    if((type >= BS_SHORT) && (type <= BS_INT)) {
        bs_attribI(type, amount, size_per_type);
        return;
    }

    bs_Batch *batch = curr_batch;

    glEnableVertexAttribArray(batch->attrib_count);
    glVertexAttribPointer(batch->attrib_count++, amount, type, normalized, batch->attrib_size_bytes, (void*)batch->attrib_offset);

    batch->attrib_offset += size_per_type;
}

void bs_attribDivisor(int attrib_id, int value) {
    glVertexAttribDivisor(attrib_id, value);
}

void bs_attribInstance(int attrib_id) {
    bs_attribDivisor(attrib_id, 1);
}

void bs_bufferRange(int target, int bind_point, int buffer, int offset, int size) {
    glBindBufferRange(target, bind_point, buffer, offset, size);
}

// Pushes all vertices to VRAM
void bs_pushBatch() {
    glBufferSubData(GL_ARRAY_BUFFER, 0, curr_batch->vertex_draw_count * curr_batch->attrib_size_bytes, curr_batch->vertices);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, curr_batch->index_draw_count * sizeof(int), curr_batch->indices);
}

void bs_freeBatchData() {
    free(curr_batch->vertices);
    free(curr_batch->indices);
    curr_batch->vertices = NULL;
    curr_batch->indices = NULL;
}

void bs_renderBatch(int start_index, int draw_count) {
    bs_switchShader(curr_batch->shader->id);

    bs_Uniform *view = &curr_batch->shader->uniforms[UNIFORM_VIEW];
    bs_Uniform *proj = &curr_batch->shader->uniforms[UNIFORM_PROJ];

    if(view->is_valid)
	bs_uniform_mat4(view->loc, curr_batch->camera->view);
    if(proj->is_valid)
	bs_uniform_mat4(proj->loc, curr_batch->camera->proj);

    glDrawElements(curr_batch->draw_mode, draw_count, BS_UINT, (void*)(start_index * sizeof(GLuint)));
}

void bs_clearBatch() {
    curr_batch->vertex_draw_count = 0;
    curr_batch->index_draw_count = 0;
}

int bs_batchSize() {
    return curr_batch->index_draw_count;
}

/* --- FRAMEBUFFERS --- */
void bs_framebufResizeCheck() {
    bs_Framebuf *framebuf = curr_framebuf;

    if(framebuf->buf_count < framebuf->buf_alloc)
	return;

    framebuf->buf_alloc += 2;
    framebuf->bufs = realloc(framebuf->bufs, framebuf->buf_alloc * sizeof(bs_Texture));
}

void bs_framebuf(bs_Framebuf *framebuf, bs_ivec2 dim) {
    curr_framebuf = framebuf;

    framebuf->dim = dim;
    framebuf->clear = GL_DEPTH_BUFFER_BIT;
    framebuf->depth_index = 0;
    framebuf->buf_count = 0;
    framebuf->buf_alloc = 4;
    framebuf->bufs = malloc(framebuf->buf_alloc * sizeof(bs_Texture));

    glGenFramebuffers(1, &framebuf->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuf->FBO);
}

void bs_setBuffer(int attachment, bs_Texture buf) {
    bs_Framebuf *framebuf = curr_framebuf;

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D, buf.id, 0);
    framebuf->bufs[framebuf->buf_count] = buf;
}

void bs_attachBuffer(int attachment, bs_Texture buf) {
    bs_framebufResizeCheck();

    bs_Framebuf *framebuf = curr_framebuf;
    framebuf->clear |= GL_COLOR_BUFFER_BIT;

    bs_setBuffer(attachment, buf);
    framebuf->buf_count++;
}

void bs_attachColorbufferType(int attachment, int type) {
    bs_Framebuf *framebuf = curr_framebuf;
    bs_Texture tex;

    switch(type) {
	case GL_RGBA    : bs_textureRGBA(&tex, framebuf->dim); break;
	case GL_RGBA16F : bs_textureRGBA16f(&tex, framebuf->dim); break;
	case GL_RGBA32F : bs_textureRGBA32f(&tex, framebuf->dim); break;
	default: return;
    }

    bs_attachBuffer(attachment, tex);
}

void bs_attachColorbuffer16(int attachment) {
    bs_attachColorbufferType(attachment, GL_RGBA16F);
}

void bs_attachColorbuffer32(int attachment) {
    bs_attachColorbufferType(attachment, GL_RGBA32F);
}

void bs_attachColorbuffer(int attachment) { 
    bs_attachColorbufferType(attachment, GL_RGBA);
}

void bs_attachRenderbuffer() {
    bs_Framebuf *framebuf = curr_framebuf;

    glGenRenderbuffers(1, &framebuf->RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, framebuf->RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebuf->dim.x, framebuf->dim.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebuf->RBO); 
}

void bs_attachDepthBufferType(int type) {
    bs_framebufResizeCheck();

    bs_Framebuf *framebuf = curr_framebuf;
    bs_Texture *tex = framebuf->bufs + framebuf->buf_count;
    framebuf->clear |= GL_DEPTH_BUFFER_BIT;
    framebuf->depth_index = framebuf->buf_count;
    framebuf->buf_count++;

    if(tex->type == BS_CUBEMAP) {
	bs_depthCube(tex, framebuf->dim.x);
	glFramebufferTexture(GL_FRAMEBUFFER, type, tex->id, 0);
	return;
    }

    if(type == GL_DEPTH_STENCIL_ATTACHMENT) {
	bs_depthStencil(tex, framebuf->dim);
	framebuf->clear |= GL_STENCIL_BUFFER_BIT;
    } else {
	bs_depth(tex, framebuf->dim);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, type, GL_TEXTURE_2D, tex->id, 0);
}

void bs_attachDepthBuffer() {
    bs_attachDepthBufferType(GL_DEPTH_ATTACHMENT);
}

void bs_attachDepthStencilBuffer() {
    bs_attachDepthBufferType(GL_DEPTH_STENCIL_ATTACHMENT);
}

void bs_setDrawBufs(int n, ...) {
    GLenum values[n];

    va_list ptr;
    va_start(ptr, n);
    for(int i = 0; i < n; i++) {
        values[i] = GL_COLOR_ATTACHMENT0 + va_arg(ptr, int);
    }
    va_end(ptr);

    glDrawBuffers(n, values);
}

void bs_startFramebufRender(bs_Framebuf *framebuf) {
    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, framebuf->dim.x, framebuf->dim.y);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuf->FBO);

    // Clear any previous drawing
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(framebuf->clear);
}

void bs_endFramebufRender() {
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    bs_ivec2 res = bs_resolution();
    glViewport(0, 0, res.x, res.y);
}

unsigned char *bs_framebufData(int x, int y, int w, int h) {
    const int rgb_size = 3;
    unsigned char *data = malloc(w * h * rgb_size);
    glReadPixels(x, y, w, h, BS_CHANNEL_RGB, BS_UBYTE, data);
    return data;
}

unsigned char *bs_screenshot() {
    bs_ivec2 res = curr_framebuf->dim;
    return bs_framebufData(0, 0, res.x, res.y);
}

void bs_screenshotFile(const char *file_name) {
    unsigned char *data = bs_screenshot();
    bs_ivec2 res = curr_framebuf->dim;
    lodepng_encode24_file(file_name, data, res.x, res.y);
    free(data);
}

void bs_polygonLine() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void bs_polygonFill() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void bs_additiveBlending() {
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);
}

void bs_defaultBlending() {
    glBlendFunc(GL_SRC_ALPHA, BS_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(BS_FUNC_ADD);
}

void bs_setGlobalVars() {
    bs_Globals globals;

    globals.res = bs_resolution();
    globals.elapsed = bs_elapsedTime();

    bs_setUniformBlockData(global_unifs, &globals);
}

void bs_modelInit();
void bs_init(int width, int height, const char *title) {
    bs_initWnd(width, height, title);
    // bs_printHardwareInfo();

    bs_lookat(&def_camera, BS_V3(0, 0, 300), BS_V3(0.0, 0.0, -1.0), BS_V3(0.0, 1.0, 0.0));
    bs_ortho(&def_camera, 0, width, 0, height, 0.01, 1000.0);

    global_unifs = bs_initUniformBlock(sizeof(bs_Globals), 0);
}

void bs_startRender(void (*render)()) {
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(BS_FACE_BACK);
    glFrontFace(BS_CCW);

    bs_defaultBlending();

    srand(time(0));

    bs_wndTick(render);
}

/* --- OpenGL Rendering Logic Abstraction --- */
void bs_enable(int val) {
    glEnable(val);
}

void bs_disable(int val) {
    glDisable(val);
}

void bs_stencilFunc(int val0, int val1, int val2) {
    glStencilFunc(val0, val1, val2);
}

void bs_depthFunc(int val) {
    glDepthFunc(val);
}

void bs_stencilMask(int val) {
    glStencilMask(val);
}

void bs_depthMask(int val) {
    glDepthMask(val);
}

void bs_colorMask(int val0, int val1, int val2, int val3) {
    glColorMask(val0, val1, val2, val3);
}

void bs_colorMaski(int i, int val0, int val1, int val2, int val3) {
    glColorMaski(i, val0, val1, val2, val3);
}

void bs_stencilOp(int val0, int val1, int val2) {
    glStencilOp(val0, val1, val2);
}

void bs_stencilOpSeparate(int val0, int val1, int val2, int val3) {
    glStencilOpSeparate(val0, val1, val2, val3);
}

void bs_cullFace(int val) {
    glCullFace(val);
}

void bs_clear(int bit_field) {
    glClear(bit_field);
}

void bs_clearStencil(int val) {
    glClearStencil(val);
}

void bs_clearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void bs_frontFace(int face) {
    glFrontFace(face);
}

void bs_blendEquation(int val) {
    glBlendEquation(val);
}

void bs_blendFunc(int val0, int val1) {
    glBlendFunc(val0, val1);
}

void bs_viewport(int x, int y, int w, int h) {
    glViewport(x, y, w, h);
}
