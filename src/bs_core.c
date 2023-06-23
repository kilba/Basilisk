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

bs_Texture def_texture;
bs_Camera def_camera;
bs_Camera *shader_camera = NULL;
bs_Batch *curr_batch = NULL;

bs_Framebuf *curr_framebuf = NULL;
bs_UniformBuffer global_unifs;

bs_Idxs def_idxs;
bs_Refs def_refs;

bs_Refs ref = { 0, 0 };
float v1_ = 0.0;
bs_vec4 v4_ = BS_V4_0;

int bs_checkError() {
    GLenum err = glGetError();
    return err; 
}

bs_Camera *bs_defCamera() {
    return &def_camera;
}

bs_Texture *bs_defTexture() {
    return &def_texture;
}

/* --- MATRICES / CAMERAS --- */
bs_Idxs bs_initIdxs() {
    return def_idxs;
}

void bs_setRef(bs_Refs r) {
    ref = r;
}

void bs_setV1_(float v) {
    v1_ = v;
}

void bs_setV4_(bs_vec4 v) {
    v4_ = v;
}

void bs_shaderCamera(bs_Camera *cam) {
    shader_camera = cam;
}

void bs_persp(bs_Camera *cam, float aspect, float fovy, float nearZ, float farZ) {
    glm_perspective(glm_rad(fovy), aspect, nearZ, farZ, cam->proj.a);
}

void bs_ortho(bs_Camera *cam, int left, int right, int bottom, int top, float nearZ, float farZ) {
    glm_ortho(left, right, bottom, top, nearZ, farZ, cam->proj.a);

    int x_res = bs_sign(left - right);
    int y_res = bs_sign(top - bottom);
}

void bs_lookat(bs_Camera *cam, bs_vec3 eye, bs_vec3 center, bs_vec3 up) {
    glm_lookat((vec3){ eye.x, eye.y, eye.z }, (vec3){ center.x, center.y, center.z }, (vec3){ up.x, up.y, up.z }, cam->view.a);
}

void bs_look(bs_Camera *cam, bs_vec3 eye, bs_vec3 dir, bs_vec3 up) {
    glm_look((vec3){ eye.x, eye.y, eye.z }, (vec3){ dir.x, dir.y, dir.z }, (vec3){ up.x, up.y, up.z }, cam->view.a);
}

/* --- BATCHED RENDERING --- */
void bs_selectBatch(bs_Batch *batch) {
    curr_batch = batch;

    glBindVertexArray(batch->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, batch->VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->EBO);

    bs_switchShader(curr_batch->shader->id);
}

void bs_bufferAppend(bs_Buffer *buf, void *data) {
    bs_bufferResizeCheck(buf, 1);

    uint8_t *dest = bs_bufferData(buf, buf->size);
    memcpy(dest, data, buf->unit_size);
}

void *bs_bufferData(bs_Buffer *buf, bs_U32 offset) {
    return (uint8_t *)(buf->data) + offset * buf->unit_size;
}

void bs_minimizeBuffer(bs_Buffer *buf) {
    buf->allocated = buf->size;
    buf->data = realloc(buf->data, buf->allocated * buf->unit_size);
}

void bs_bufferResizeCheck(bs_Buffer *buf, bs_U32 num_units) {
    if((buf->size + num_units) < buf->allocated)
	return;

    buf->allocated += BS_MAX(num_units, buf->increment);

    if(buf->realloc_ram) {
	buf->data = realloc(buf->data, buf->allocated * buf->unit_size);
	printf("REALLOCED : %p, %d\n", buf, buf->allocated * buf->unit_size / 1024);
    }

    if(buf->realloc_vram)
	glBufferData(buf->type, buf->allocated * buf->unit_size, buf->data, GL_STATIC_DRAW);
}

bs_Buffer bs_buffer(bs_U32 type, bs_U32 unit_size, bs_U32 increment, bs_U32 pre_malloc) {
    void *data = NULL;
    if(pre_malloc != 0)
	data = malloc(pre_malloc * unit_size);

    return (bs_Buffer) { 0, unit_size, 0, increment, true, type != 0, type, data };
}

void bs_batchResizeCheck(int index_count, int vertex_count) {
    bs_bufferResizeCheck(&curr_batch->vertex_buf, vertex_count);
    bs_bufferResizeCheck(&curr_batch->index_buf, index_count);
}

void bs_pushIndex(int idx) {
    idx += curr_batch->vertex_buf.size;
    memcpy(
	(int *)(curr_batch->index_buf.data) + curr_batch->index_buf.size++, 
	&idx, 
	sizeof(bs_U32)
    );
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
    bs_vec4  wei
) {
    bs_Batch *batch = curr_batch;

    uint8_t *data_ptr = bs_bufferData(&batch->vertex_buf, batch->vertex_buf.size);
    uint8_t *sizes = batch->shader->attrib_sizes;
    
    bs_pushAttrib(&data_ptr, &pos, sizes[0]);
    bs_pushAttrib(&data_ptr, &tex, sizes[1]);
    bs_pushAttrib(&data_ptr, &col, sizes[2]);
    bs_pushAttrib(&data_ptr, &nor, sizes[3]);
    bs_pushAttrib(&data_ptr, &bid, sizes[4]);
    bs_pushAttrib(&data_ptr, &wei, sizes[5]);
    bs_pushAttrib(&data_ptr, &ref, sizes[6]);
    bs_pushAttrib(&data_ptr, &v4_, sizes[7]);
    bs_pushAttrib(&data_ptr, &v1_, sizes[8]);
    
    curr_batch->vertex_buf.size++;
} 

int bs_pushQuadFlipped(bs_vec3 p0, bs_vec3 p1, bs_vec3 p2, bs_vec3 p3, bs_RGBA col) {
    bs_batchResizeCheck(6, 4);

    bs_pushIndexVa(6, 0, 1, 2, 2, 1, 3);

    bs_pushVertex(p0, (bs_vec2){ 0.0, 1.0 }, BS_V3_0, col, BS_IV4_0, BS_V4_0);
    bs_pushVertex(p1, (bs_vec2){ 1.0, 1.0 }, BS_V3_0, col, BS_IV4_0, BS_V4_0);
    bs_pushVertex(p2, (bs_vec2){ 0.0, 0.0 }, BS_V3_0, col, BS_IV4_0, BS_V4_0);
    bs_pushVertex(p3, (bs_vec2){ 1.0, 0.0 }, BS_V3_0, col, BS_IV4_0, BS_V4_0);
    
    return curr_batch->index_buf.size;    
}    
    
int bs_pushQuad(bs_vec3 p0, bs_vec3 p1, bs_vec3 p2, bs_vec3 p3, bs_RGBA col) {    
    bs_batchResizeCheck(6, 4);    
    
    bs_pushIndexVa(6, 0, 1, 2, 2, 1, 3);    
    
    bs_pushVertex(p0, (bs_vec2){ 0.0, 0.0 }, BS_V3_0, col, BS_IV4_0, BS_V4_0); 
    bs_pushVertex(p1, (bs_vec2){ 1.0, 0.0 }, BS_V3_0, col, BS_IV4_0, BS_V4_0); 
    bs_pushVertex(p2, (bs_vec2){ 0.0, 1.0 }, BS_V3_0, col, BS_IV4_0, BS_V4_0); 
    bs_pushVertex(p3, (bs_vec2){ 1.0, 1.0 }, BS_V3_0, col, BS_IV4_0, BS_V4_0); 
    
    return curr_batch->index_buf.size;    
}    
    
int bs_pushRectCoord(bs_vec3 pos, bs_vec2 dim, bs_vec2 tex0, bs_vec2 tex1, bs_RGBA col) {
    bs_batchResizeCheck(6, 4);    
    
    dim.x += pos.x;    
    dim.y += pos.y;    

    bs_pushIndexVa(6, 0, 1, 2, 2, 1, 3);

    bs_pushVertex((bs_vec3){ pos.x, pos.y, pos.z }, (bs_vec2){ tex0.x, tex1.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0);
    bs_pushVertex((bs_vec3){ dim.x, pos.y, pos.z }, (bs_vec2){ tex1.x, tex1.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0);
    bs_pushVertex((bs_vec3){ pos.x, dim.y, pos.z }, (bs_vec2){ tex0.x, tex0.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0);
    bs_pushVertex((bs_vec3){ dim.x, dim.y, pos.z }, (bs_vec2){ tex1.x, tex0.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0);

    return curr_batch->index_buf.size;
}

int bs_pushRectRotated(bs_vec3 pos, bs_vec2 dim, float angle, bs_RGBA col) {
    bs_batchResizeCheck(6, 4);
    bs_Texture *tex = bs_selectedTexture();

    dim.x += pos.x;
    dim.y += pos.y;

    bs_pushIndexVa(6, 0, 1, 2, 2, 1, 3);

    bs_vec3 p0, p1, p2, p3;
    p0 = BS_V2_Z(bs_v2rot(bs_v2(pos.x, pos.y), bs_v2(pos.x, pos.y), angle), pos.z);
    p1 = BS_V2_Z(bs_v2rot(bs_v2(dim.x, pos.y), bs_v2(pos.x, pos.y), angle), pos.z);
    p2 = BS_V2_Z(bs_v2rot(bs_v2(pos.x, dim.y), bs_v2(pos.x, pos.y), angle), pos.z);
    p3 = BS_V2_Z(bs_v2rot(bs_v2(dim.x, dim.y), bs_v2(pos.x, pos.y), angle), pos.z);

    bs_vec2 tex0, tex1;
    tex0.x = tex->texw * (float)tex->frame.x;
    tex1.y = tex->texh * (float)tex->frame.y;
    tex1.x = tex0.x + tex->texw;
    tex0.y = tex0.y + tex->texh;

    bs_pushVertex(p0, (bs_vec2){ tex0.x, tex1.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0);
    bs_pushVertex(p1, (bs_vec2){ tex1.x, tex1.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0);
    bs_pushVertex(p2, (bs_vec2){ tex0.x, tex0.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0);
    bs_pushVertex(p3, (bs_vec2){ tex1.x, tex0.y }, BS_V3_0, col, BS_IV4_0, BS_V4_0);

    return curr_batch->index_buf.size;
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

    bs_pushVertex(pos1, bs_v2(0.0, 0.0), BS_V3_0, color, BS_IV4_0, BS_V4_0);
    bs_pushVertex(pos2, bs_v2(1.0, 0.0), BS_V3_0, color, BS_IV4_0, BS_V4_0);
    bs_pushVertex(pos3, bs_v2(0.0, 1.0), BS_V3_0, color, BS_IV4_0, BS_V4_0);

    return curr_batch->index_buf.size;
}

int bs_pushLine(bs_vec3 start, bs_vec3 end, bs_RGBA color) {
    bs_batchResizeCheck(2, 2);
    bs_pushIndexVa(2, 0, 1),

    bs_pushVertex(start, bs_v2(0.0, 0.0), BS_V3_0, color, BS_IV4_0, BS_V4_0);
    bs_pushVertex(end, bs_v2(1.0, 0.0), BS_V3_0, color, BS_IV4_0, BS_V4_0);

    return curr_batch->index_buf.size;
}

int bs_pushPoint(bs_vec3 pos, bs_RGBA color) {
    bs_batchResizeCheck(1, 1);
    bs_pushIndexVa(1, 0);
    bs_pushVertex(pos, BS_V2_0, BS_V3_0, color, BS_IV4_0, BS_V4_0);

    return curr_batch->index_buf.size;
}

int bs_pushAABB(bs_aabb aabb, bs_RGBA color) {
    bs_batchResizeCheck(36, 8);
    bs_pushIndexVa(36,
	2, 7, 6, 2, 3, 7, // Bottom
	0, 4, 5, 0, 5, 1, // Top
	0, 2, 6, 0, 6, 4, // Left	
	1, 7, 3, 1, 5, 7, // Right
	0, 1, 2, 2, 1, 3, // Front
	4, 6, 7, 4, 7, 5  // Back
    );

    // TODO: Set correct normals
    bs_pushVertex(bs_v3(aabb.min.x, aabb.min.y, aabb.min.z), bs_v2(0.0, 0.0), bs_v3(0.0, 0.0, 0.0), color, BS_IV4_0, BS_V4_0);
    bs_pushVertex(bs_v3(aabb.max.x, aabb.min.y, aabb.min.z), bs_v2(0.0, 0.0), bs_v3(0.0, 0.0, 0.0), color, BS_IV4_0, BS_V4_0);
    bs_pushVertex(bs_v3(aabb.min.x, aabb.max.y, aabb.min.z), bs_v2(0.0, 0.0), bs_v3(0.0, 0.0, 0.0), color, BS_IV4_0, BS_V4_0);
    bs_pushVertex(bs_v3(aabb.max.x, aabb.max.y, aabb.min.z), bs_v2(0.0, 0.0), bs_v3(0.0, 0.0, 0.0), color, BS_IV4_0, BS_V4_0);
    
    bs_pushVertex(bs_v3(aabb.min.x, aabb.min.y, aabb.max.z), bs_v2(0.0, 0.0), bs_v3(0.0, 0.0, 0.0), color, BS_IV4_0, BS_V4_0);
    bs_pushVertex(bs_v3(aabb.max.x, aabb.min.y, aabb.max.z), bs_v2(0.0, 0.0), bs_v3(0.0, 0.0, 0.0), color, BS_IV4_0, BS_V4_0);
    bs_pushVertex(bs_v3(aabb.min.x, aabb.max.y, aabb.max.z), bs_v2(0.0, 0.0), bs_v3(0.0, 0.0, 0.0), color, BS_IV4_0, BS_V4_0);
    bs_pushVertex(bs_v3(aabb.max.x, aabb.max.y, aabb.max.z), bs_v2(0.0, 0.0), bs_v3(0.0, 0.0, 0.0), color, BS_IV4_0, BS_V4_0);

    return curr_batch->index_buf.size;
}

int bs_pushPrim(bs_Prim *prim, int num_vertices, int num_indices) {
    bs_batchResizeCheck(BS_MAX(prim->index_count, num_indices), BS_MAX(prim->vertex_count, num_vertices));

    bs_pushIndices(prim->indices, prim->index_count);

    bs_Refs original_ref = ref;

    if(original_ref.count == prim->parent->parent->material_count) {
	bs_setRef((bs_Refs){ original_ref.value + prim->material_idx, 1 });
    } else {
	bs_setRef(prim->parent->parent->refs);
    }

    float *vertex = prim->vertices;
    for(int i = 0; i < prim->vertex_count; i++, vertex += prim->vertex_size) {
        bs_pushVertex(
            *(bs_vec3  *)(vertex + 0),
	    *(bs_vec2  *)(vertex + prim->offset_tex),
	    *(bs_vec3  *)(vertex + prim->offset_nor),
	    BS_WHITE,
	    *(bs_ivec4 *)(vertex + prim->offset_bid),
	    *(bs_vec4  *)(vertex + prim->offset_wei)
        );
    }

    bs_setRef(original_ref);
    return curr_batch->index_buf.size;
}

int bs_pushMesh(bs_Mesh *mesh, int num_vertices, int num_indices) {
    int ret = 0;
    for(int i = 0; i < mesh->prim_count; i++) {
        bs_Prim *prim = &mesh->prims[i];
        ret += bs_pushPrim(prim, num_vertices, num_indices);
    }
    return ret;
}

int bs_pushModel(bs_Model *model) {
    int ret = 0;
    for(int i = 0; i < model->mesh_count; i++) {
        ret += bs_pushMesh(&model->meshes[i], model->vertex_count, model->index_count);
    }
    return ret;
}

bs_U32 bs_batchOffset() {
    return curr_batch->index_buf.size;
}

void bs_batch(bs_Batch *batch, bs_Shader *shader) {
    // Default values
    memset(batch, 0, sizeof(bs_Batch));
    batch->draw_mode = BS_TRIANGLES;
    batch->camera = &def_camera;
    batch->shader = shader;
    batch->use_indices = true;

    // Create buffer/array objects
    glGenVertexArrays(1, &batch->VAO);
    glGenBuffers(1, &batch->VBO);
    glGenBuffers(1, &batch->EBO);

    bs_selectBatch(batch);

    // Attribute setup
    struct AttribData {
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
        { BS_UINT , 1, sizeof(int  )   , false }, /* Index */
        { BS_FLOAT, 4, sizeof(bs_vec4) , false }, /* V4_ Attrib */
        { BS_FLOAT, 1, sizeof(float)   , false }, /* V1_ Attrib */
    };
    int total_attrib_count = sizeof(attrib_data) / sizeof(struct AttribData);

    // Calculate attrib sizes
    int i = 0, j = 1;
    int attrib_size = 0;
    for(; i < total_attrib_count; i++, j *= 2) {
        struct AttribData *data = &attrib_data[i];

        if((batch->shader->attribs & j) == j)
            attrib_size += data->size;
    }

    // Add attributes
    i = 0; j = 1;
    for(; i < total_attrib_count; i++, j *= 2) {
        struct AttribData *data = &attrib_data[i];

        if((batch->shader->attribs & j) == j)
            bs_attrib(data->type, data->count, data->size, attrib_size, data->normalized);
    }
    
    batch->vertex_buf = bs_buffer(GL_ARRAY_BUFFER, attrib_size, BS_BATCH_INCR_BY, 0);
    batch->index_buf  = bs_buffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(bs_U32), BS_BATCH_INCR_BY, 0);
}

void bs_batchRawData(void *vertex_data, void *index_data, int vertex_size, int index_size) {
    glBufferData(GL_ARRAY_BUFFER, vertex_size, vertex_data, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_size, index_data, GL_STATIC_DRAW);
}

void bs_attribI(const int type, unsigned int amount, size_t size_per_type, size_t attrib_size) {
    bs_Batch *batch = curr_batch;

    glEnableVertexAttribArray(batch->attrib_count);
    glVertexAttribIPointer(batch->attrib_count++, amount, type, attrib_size, (void*)batch->attrib_offset);

    batch->attrib_offset += size_per_type;
}

void bs_attrib(const int type, unsigned int amount, size_t size_per_type, size_t attrib_size, bool normalized) {
    if((type >= BS_SHORT) && (type <= BS_UINT)) {
        bs_attribI(type, amount, size_per_type, attrib_size);
        return;
    }

    bs_Batch *batch = curr_batch;

    glEnableVertexAttribArray(batch->attrib_count);
    glVertexAttribPointer(batch->attrib_count++, amount, type, normalized, attrib_size, (void*)batch->attrib_offset);

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

// Pushes all vertex_buf.data to VRAM
void bs_pushBatch() {
    glBufferSubData(GL_ARRAY_BUFFER, 0, curr_batch->vertex_buf.size * curr_batch->vertex_buf.unit_size, curr_batch->vertex_buf.data);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, curr_batch->index_buf.size * sizeof(int), curr_batch->index_buf.data);
}

void bs_minimizeBatch() {
    bs_minimizeBuffer(&curr_batch->vertex_buf);
    bs_minimizeBuffer(&curr_batch->index_buf);
}

void bs_freeBatchData() {
    free(curr_batch->vertex_buf.data);
    free(curr_batch->index_buf.data);
    curr_batch->vertex_buf.data = NULL;
    curr_batch->index_buf.data = NULL;
}

void bs_renderBatchData() {
    bs_switchShader(curr_batch->shader->id);

    bs_Uniform *view = &curr_batch->shader->uniforms[UNIFORM_VIEW];
    bs_Uniform *proj = &curr_batch->shader->uniforms[UNIFORM_PROJ];

    if(view->is_valid)
	bs_uniformM4(view->loc, curr_batch->camera->view);
    if(proj->is_valid)
	bs_uniformM4(proj->loc, curr_batch->camera->proj);
}

void bs_renderBatch(int start_index, int draw_count) {
    bs_renderBatchData();

    if(curr_batch->use_indices) {
	glDrawElements(curr_batch->draw_mode, draw_count, BS_UINT, (void*)(start_index * sizeof(GLuint)));
    } else {
	glDrawArrays(curr_batch->draw_mode, start_index, draw_count);
    }
}

void bs_clearBatch() {
    curr_batch->vertex_buf.size = 0;
    curr_batch->index_buf.size = 0;
}

int bs_batchSize() {
    if(curr_batch->use_indices) {
	return curr_batch->index_buf.size;
    } else {
	return curr_batch->vertex_buf.size;
    }
}

/* --- FRAMEBUFFERS --- */
void bs_framebufResizeCheck() {
    bs_Framebuf *framebuf = curr_framebuf;

    bs_bufferResizeCheck(&framebuf->buf, 1);
}

void bs_framebuf(bs_Framebuf *framebuf, bs_ivec2 dim) {
    curr_framebuf = framebuf;

    framebuf->dim = dim;
    framebuf->clear = GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
    framebuf->buf = bs_buffer(0, sizeof(bs_Texture), 4, 4);

    glGenFramebuffers(1, &framebuf->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuf->FBO);
}

void bs_setBuf(int type, int idx, bs_Texture buf) {
    bs_Framebuf *framebuf = curr_framebuf;

    glFramebufferTexture2D(GL_FRAMEBUFFER, type, GL_TEXTURE_2D, buf.id, 0);
    memcpy((bs_Texture *)(framebuf->buf.data) + idx, &buf, sizeof(bs_Texture));
}

void bs_attachBufExisting(bs_Texture buf, int type) {
    bs_Framebuf *framebuf = curr_framebuf;

    bs_framebufResizeCheck();
    bs_setBuf(type, framebuf->buf.size, buf);

    framebuf->buf.size++;
}

void bs_attachBuf(void (*tex_func)(bs_Texture *texture, bs_ivec2 dim)) {
    bs_Framebuf *framebuf = curr_framebuf;
    bs_framebufResizeCheck();
    
    bs_Texture *buf = (bs_Texture *)(framebuf->buf.data) + framebuf->buf.size;
    tex_func(buf, framebuf->dim);

    // Add attachment offset only to BS_COLOR (GL_COLOR_ATTACHMENT0)
    int attachment = buf->attachment + ((buf->attachment == BS_COLOR) ? framebuf->buf.size : 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, buf->id, 0);
    framebuf->buf.size++;
}

void bs_attachRenderbuf() {
    bs_Framebuf *framebuf = curr_framebuf;

    glGenRenderbuffers(1, &framebuf->RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, framebuf->RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebuf->dim.x, framebuf->dim.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebuf->RBO); 
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

void bs_selectFramebuf(bs_Framebuf *framebuf) {
    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, framebuf->dim.x, framebuf->dim.y);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuf->FBO);

    // Clear any previous drawing
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(framebuf->clear);
}

void bs_pushFramebuf() {
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

void bs_stencilDefault() {
    bs_stencilFunc(BS_ALWAYS, 0, 0xFF);
    bs_stencilOp(BS_KEEP, BS_KEEP, BS_KEEP);
}

void bs_stencilEqZero() {
    bs_stencilFunc(BS_EQUAL, 0, 0xFF);
    bs_stencilOp(BS_KEEP, BS_KEEP, BS_KEEP);
}

void bs_stencilEqNonZero() {
    bs_stencilFunc(BS_NOTEQUAL, 0, 0xFF);
    bs_stencilOp(BS_KEEP, BS_KEEP, BS_KEEP);
}

void bs_polygonLine() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void bs_polygonFill() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void bs_disableDepth() {
    glDepthMask(false);
}

void bs_enableDepth() {
    glDepthMask(true);
}

void bs_disableColorsI(int num, ...) {
    va_list ptr;
    va_start(ptr, num);
 
    for (int i = 0; i < num; i++)
	bs_disableColorI(va_arg(ptr, int));
 
    va_end(ptr);
}

void bs_enableColorsI(int num, ...) {
    va_list ptr;
    va_start(ptr, num);
 
    for (int i = 0; i < num; i++)
	bs_enableColorI(va_arg(ptr, int));
 
    va_end(ptr);
}

void bs_disableColorI(int i) {
    glColorMaski(i, false, false, false, false);
}

void bs_enableColorI(int i) {
    glColorMaski(i, true, true, true, true);
}

void bs_disableColor() {
    glColorMask(false, false, false, false);
}

void bs_enableColor() {
    glColorMask(true, true, true, true);
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

    // TODO: actual camera system, update camera position for every batch
    bs_mat4 view_inv;
    glm_mat4_inv((shader_camera != NULL ? shader_camera : curr_batch->camera)->view.a, view_inv.a);

    globals.res = bs_resolution();
    globals.elapsed = bs_elapsedTime();
    globals.cam_pos = bs_v3(view_inv.a[3][0], view_inv.a[3][1], view_inv.a[3][2]);

    bs_setUniformBlockData(global_unifs, &globals);
}

void bs_modelInit();
void bs_init(bs_U32 width, bs_U32 height, const char *title) {
    bs_initWnd(width, height, title);

    bs_lookat(&def_camera, bs_v3(0, 0, 300), bs_v3(0.0, 0.0, -1.0), bs_v3(0.0, 1.0, 0.0));
    bs_ortho(&def_camera, 0, width, 0, height, 0.01, 1000.0);

    bs_shaderBufs();

    bs_RGBA data = BS_WHITE;
    bs_textureDataRGBA(&def_texture, (unsigned char *)&data, BS_IV2(1, 1));

    def_idxs.model = bs_shaderModelInit(BS_MAT4_IDENTITY);
    def_idxs.frame = 0;
    def_idxs.texture_handle = def_texture.handle;

    def_refs = bs_shaderReferences(def_idxs);

    // TODO: Extract to bs_shaderBufs
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
void bs_lineWidth(float w) {
    glLineWidth(w);
}

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
