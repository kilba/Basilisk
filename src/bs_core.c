// GL
#include "bs_types.h"
#include <glad/glad.h>
#include <cglm/cglm.h>

// Basilisk
#include <bs_shaders.h>
#include <bs_textures.h>
#include <bs_core.h>
#include <bs_math.h>
#include <bs_wnd.h>

// TODO: Ifdef debug
#include <bs_debug.h>

// STD
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <lodepng.h>

#ifdef _WIN32
    #include <windows.h>
    #include <objidl.h>
#endif

#include <time.h>

// Shaders
bs_Shader texture_shader;

bs_Camera def_camera;
bs_Batch *curr_batch;

bs_Framebuffer *curr_framebuffer = NULL;
bs_UniformBuffer global_unifs;

// TODO: Extract to bs_debug.c
void bs_printHardwareInfo() {
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    bs_print(BS_CLE, "%s\n", vendor);
    bs_print(BS_CLE, "%s\n", renderer);
}

#ifdef _WIN32
    BITMAPINFOHEADER createBitmapHeader(int width, int height) {
        BITMAPINFOHEADER bi;

        // create a bitmap
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = width;
        bi.biHeight = -height;
        bi.biPlanes = 1;
        bi.biBitCount = 24;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        return bi;
    }

    unsigned char* getScreenTexture(int x, int y, int w, int h) {
        HWND hWnd = GetDesktopWindow();

        HDC hwindowDC = GetDC(hWnd);
        HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
        SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

        int screenx = x;
        int screeny = y;
        int width = w;
        int height = h;

        HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
        BITMAPINFOHEADER bi = createBitmapHeader(width, height);

        SelectObject(hwindowCompatibleDC, hbwindow);

        BITMAP bmp;
        GetObject(hbwindow, sizeof(BITMAP), (LPVOID)&bmp);

        DWORD dwBmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;
        HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
        unsigned char* lpbitmap = (unsigned char*)GlobalLock(hDIB);
 
        // copy from the window device context to the bitmap device context
        StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screenx, screeny, width, height, SRCCOPY);   //change SRCCOPY to NOTSRCCOPY for wacky colors !
        GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

        // GlobalFree(hDIB);
        DeleteDC(hwindowCompatibleDC);
        ReleaseDC(hWnd, hwindowDC);

        // return lpbitmap;
        return bmp.bmBits;
    }
#endif

/* --- MATRICES --- */
void bs_ortho(bs_mat4 mat, int left, int right, int bottom, int top, float nearZ, float farZ) {
    glm_ortho(left, right, bottom, top, nearZ, farZ, mat);

    int x_res = bs_sign(left - right);
    int y_res = bs_sign(top - bottom);
}

void bs_lookat(bs_mat4 mat, bs_vec3 eye, bs_vec3 center, bs_vec3 up) {
    glm_lookat((vec3){ eye.x, eye.y, eye.z }, (vec3){ center.x, center.y, center.z }, (vec3){ up.x, up.y, up.z }, mat);
}

void bs_look(bs_mat4 mat, bs_vec3 eye, bs_vec3 dir, bs_vec3 up) {
    glm_look((vec3){ eye.x, eye.y, eye.z }, (vec3){ dir.x, dir.y, dir.z }, (vec3){ up.x, up.y, up.z }, mat);
}

void bs_persp(bs_mat4 mat, float aspect, float fovy, float nearZ, float farZ) {
    glm_perspective(glm_rad(fovy), aspect, nearZ, farZ, mat);
}

/* --- UNBATCHED RENDERING --- */

/* --- BATCHED RENDERING --- */
void bs_selectBatchNoShader(bs_Batch *batch) {
    curr_batch = batch;

    glBindVertexArray(batch->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, batch->VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->EBO);
}

void bs_selectBatch(bs_Batch *batch) {
    bs_selectBatchNoShader(batch);
    bs_switchShader(curr_batch->shader->id);
}

void bs_pushVertex(
    bs_vec3  pos,
    bs_vec2  tex_coord,
    bs_vec3  normal,
    bs_RGBA  color,
    bs_ivec4 bone_ids,
    bs_vec4  weights,
    bs_vec4  attrib_vec4) {
    bs_Batch *batch = curr_batch;

    unsigned char *data_ptr = (unsigned char *)batch->vertices + batch->vertex_draw_count * batch->attrib_size_bytes;

    bool has_position  = ((batch->shader->attribs & BS_POSITION)  == BS_POSITION);
    bool has_tex_coord = /*(tex_coord != 0)   * */((batch->shader->attribs & BS_TEX_COORD) == BS_TEX_COORD);
    bool has_normal    = ((batch->shader->attribs & BS_NORMAL)    == BS_NORMAL);
    bool has_color     = ((batch->shader->attribs & BS_COLOR)     == BS_COLOR);
    bool has_bone_ids  = ((batch->shader->attribs & BS_BONE_IDS)  == BS_BONE_IDS);
    bool has_weights   = ((batch->shader->attribs & BS_WEIGHTS)   == BS_WEIGHTS);
    bool has_attr_vec4 = ((batch->shader->attribs & BS_ATTR_VEC4) == BS_ATTR_VEC4);

    memcpy(data_ptr, &pos, sizeof(bs_vec3) * has_position);
    data_ptr += sizeof(bs_vec3) * has_position;

    memcpy(data_ptr, &tex_coord, sizeof(bs_vec2) * has_tex_coord);
    data_ptr += sizeof(bs_vec2) * has_tex_coord;

    memcpy(data_ptr, &color, sizeof(bs_RGBA) * has_color);
    data_ptr += sizeof(bs_RGBA) * has_color;

    memcpy(data_ptr, &normal, sizeof(bs_vec3) * has_normal); 
    data_ptr += sizeof(bs_vec3) * has_normal;

    memcpy(data_ptr, &bone_ids, sizeof(bs_ivec4) * has_bone_ids); 
    data_ptr += sizeof(bs_ivec4) * has_bone_ids;

    memcpy(data_ptr, &weights, sizeof(bs_vec4) * has_weights); 
    data_ptr += sizeof(bs_vec4) * has_weights;

    memcpy(data_ptr, &attrib_vec4, sizeof(bs_vec4) * has_attr_vec4); 
    data_ptr += sizeof(bs_vec4) * has_attr_vec4;

    curr_batch->vertex_draw_count++;
} 

void bs_pushQuad(bs_vec3 p0, bs_vec3 p1, bs_vec3 p2, bs_vec3 p3, bs_RGBA col) {
    int indices[] = {
        curr_batch->vertex_draw_count+0, curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2,
        curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2, curr_batch->vertex_draw_count+3,
    };

    memcpy(&curr_batch->indices[curr_batch->index_draw_count], indices, 6 * sizeof(int));
    
    bs_pushVertex(p0, (bs_vec2){ 0.0, 0.0 }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Bottom Left
    bs_pushVertex(p1, (bs_vec2){ 1.0, 0.0 }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Bottom right
    bs_pushVertex(p2, (bs_vec2){ 0.0, 1.0 }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Top Left
    bs_pushVertex(p3, (bs_vec2){ 1.0, 1.0 }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Top Right

    curr_batch->index_draw_count += 6;
}

void bs_pushTex2DFlipped(bs_vec3 pos, bs_vec2 dim, bs_RGBA col) {
    dim.x += pos.x;
    dim.y += pos.y;

    int indices[] = {
        curr_batch->vertex_draw_count+0, curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2,
        curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2, curr_batch->vertex_draw_count+3,
    };

    memcpy(&curr_batch->indices[curr_batch->index_draw_count], indices, 6 * sizeof(int));

    bs_pushVertex((bs_vec3){ pos.x, pos.y, pos.z }, (bs_vec2){ 0.0, 1.0 }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Bottom Left
    bs_pushVertex((bs_vec3){ dim.x, pos.y, pos.z }, (bs_vec2){ 1.0, 1.0 }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Bottom right
    bs_pushVertex((bs_vec3){ pos.x, dim.y, pos.z }, (bs_vec2){ 0.0, 0.0 }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Top Left
    bs_pushVertex((bs_vec3){ dim.x, dim.y, pos.z }, (bs_vec2){ 1.0, 0.0 }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Top Right

    curr_batch->index_draw_count += 6;
}

void bs_pushTex2D(bs_vec3 pos, bs_vec2 dim, bs_RGBA col) {
    bs_Tex2D *tex = bs_selectedTexture();

    dim.x += pos.x;
    dim.y += pos.y;

    int indices[] = {
        curr_batch->vertex_draw_count+0, curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2,
        curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2, curr_batch->vertex_draw_count+3,
    };

    memcpy(&curr_batch->indices[curr_batch->index_draw_count], indices, 6 * sizeof(int));

    float texx = tex->texw * (float)tex->frame.x;
    float texy = tex->texh * (float)tex->frame.y;
    float texw = texx + tex->texw;
    float texh = texy + tex->texh;

    printf("%f\n", texw);
    bs_pushVertex((bs_vec3){ pos.x, pos.y, pos.z }, (bs_vec2){ texx, texy }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Bottom Left
    bs_pushVertex((bs_vec3){ dim.x, pos.y, pos.z }, (bs_vec2){ texw, texy }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Bottom right
    bs_pushVertex((bs_vec3){ pos.x, dim.y, pos.z }, (bs_vec2){ texx, texh }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Top Left
    bs_pushVertex((bs_vec3){ dim.x, dim.y, pos.z }, (bs_vec2){ texw, texh }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Top Right

    curr_batch->index_draw_count += 6;
}

void bs_pushRect(bs_vec3 pos, bs_vec2 dim, bs_RGBA col) {
    dim.x += pos.x;
    dim.y += pos.y;

    int indices[] = {
        curr_batch->vertex_draw_count+0, curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2,
        curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2, curr_batch->vertex_draw_count+3,
    };

    memcpy(&curr_batch->indices[curr_batch->index_draw_count], indices, 6 * sizeof(int));

    const float white = 0.9999;
    bs_pushVertex((bs_vec3){ pos.x, pos.y, pos.z }, (bs_vec2){ white, white }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Bottom Left
    bs_pushVertex((bs_vec3){ dim.x, pos.y, pos.z }, (bs_vec2){ white, white }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Bottom right
    bs_pushVertex((bs_vec3){ pos.x, dim.y, pos.z }, (bs_vec2){ white, white }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Top Left
    bs_pushVertex((bs_vec3){ dim.x, dim.y, pos.z }, (bs_vec2){ white, white }, bs_vec3_0, col, bs_ivec4_0, bs_vec4_0, bs_vec4_0); // Top Right

    curr_batch->index_draw_count += 6;
}

void bs_pushTriangle(bs_vec3 pos1, bs_vec3 pos2, bs_vec3 pos3, bs_RGBA color) {
    int indices[] = {
        curr_batch->vertex_draw_count+0, curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2,
    };
    memcpy(&curr_batch->indices[curr_batch->index_draw_count], indices, 3 * sizeof(int));

    bs_pushVertex(pos1, (bs_vec2){ 0.0, 0.0 }, bs_vec3_0, color, bs_ivec4_0, bs_vec4_0, bs_vec4_0);
    bs_pushVertex(pos2, (bs_vec2){ 1.0, 0.0 }, bs_vec3_0, color, bs_ivec4_0, bs_vec4_0, bs_vec4_0);
    bs_pushVertex(pos3, (bs_vec2){ 0.0, 1.0 }, bs_vec3_0, color, bs_ivec4_0, bs_vec4_0, bs_vec4_0);

    curr_batch->index_draw_count += 3;
}

void bs_pushLine(bs_vec3 start, bs_vec3 end, bs_RGBA color) {
    bs_pushTriangle(start, end, end, color);
}

void bs_pushPrim(bs_Prim *prim) {
    for(int i = 0; i < prim->index_count; i++) {
        curr_batch->indices[curr_batch->index_draw_count+i] = prim->indices[i] + curr_batch->vertex_draw_count;
    }

    for(int i = 0; i < prim->vertex_count; i++) {
        const float white = 0.9999;
        bs_vec2 tex_coord = { white, white };

        if(prim->material.tex != NULL) {
            // TODO: These values are constant, unnecessary to set them every frame
            tex_coord.x = prim->vertices[i].tex_coord.x;/* + prim->material.tex->tex_x;*/
            tex_coord.y = prim->vertices[i].tex_coord.y;/* + prim->material.tex->tex_y;*/
        }

        bs_pushVertex(
            prim->vertices[i].position, 
            tex_coord, 
            prim->vertices[i].normal, 
            prim->material.col, 
            prim->vertices[i].bone_ids, 
            prim->vertices[i].weights, 
            bs_vec4_0
        );
    }

    curr_batch->index_draw_count += prim->index_count;
}

void bs_pushMesh(bs_Mesh *mesh) {
    for(int i = 0; i < mesh->prim_count; i++) {
        bs_Prim *prim = &mesh->prims[i];
        bs_pushPrim(prim);
    }
}

void bs_pushModel(bs_Model *model) {
    for(int i = 0; i < model->mesh_count; i++) {
        bs_pushMesh(&model->meshes[i]);
    }
}

void bs_changeBatchBufferSize(bs_Batch *batch, int index_count) {
    batch->vertices = realloc(batch->vertices, index_count * sizeof(bs_Vertex));    
    batch->indices  = realloc(batch->indices , index_count * sizeof(int));

    glBufferData(GL_ARRAY_BUFFER, sizeof(bs_Vertex)  * index_count, batch->vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * index_count, batch->indices, GL_STATIC_DRAW);
}

void bs_batch(bs_Batch *batch, bs_Shader *shader, int index_count) {
    // Default values
    batch->camera = &def_camera;
    batch->draw_mode = BS_TRIANGLES;
    batch->vertex_draw_count = 0;
    batch->index_draw_count = 0;
    batch->attrib_count = 0;
    batch->attrib_offset = 0;
    batch->attrib_size_bytes = 0;
    batch->shader = shader;

    if(batch->shader == NULL) {
        batch->shader = &texture_shader;
    }

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
    for(; i < total_attrib_count; i++, j*=2) {
        struct Attrib_Data *data = &attrib_data[i];

        if((batch->shader->attribs & j) == j) {
            batch->attrib_size_bytes += data->size;
        }
    }

    // Add attributes
    i = 0; j = 1;
    for(; i < total_attrib_count; i++, j*=2) {
        struct Attrib_Data *data = &attrib_data[i];

        if((batch->shader->attribs & j) == j) {
            bs_attrib(data->type, data->count, data->size, data->normalized);
        }
    }

    // Allocate buffer spaces
    batch->indices = malloc(sizeof(int) * index_count);
    batch->vertices = malloc(batch->attrib_size_bytes * index_count);
    glBufferData(GL_ARRAY_BUFFER, batch->attrib_size_bytes * index_count, NULL, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * index_count, NULL, GL_STATIC_DRAW);
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
    // Batch should already be bound at this point so binding it again is wasteful
    glBufferSubData(GL_ARRAY_BUFFER, 0, curr_batch->vertex_draw_count * curr_batch->attrib_size_bytes, curr_batch->vertices);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, curr_batch->index_draw_count * sizeof(int), curr_batch->indices);
}

void bs_freeBatchData() {
    free(curr_batch->vertices);
    free(curr_batch->indices);
    curr_batch->vertices = NULL;
    curr_batch->indices = NULL;
}

void bs_renderBatchNoShader(int start_index, int draw_count) {
    bs_Uniform *view = &curr_batch->shader->uniforms[UNIFORM_VIEW];
    bs_Uniform *proj = &curr_batch->shader->uniforms[UNIFORM_PROJ];

    if(view->is_valid)
	bs_uniform_mat4(view->loc, curr_batch->camera->view);
    if(proj->is_valid)
	bs_uniform_mat4(proj->loc, curr_batch->camera->proj);

    glDrawElements(curr_batch->draw_mode, draw_count, BS_UINT, (void*)(start_index * 6 * sizeof(GLuint)));
}

void bs_renderBatch(int start_index, int draw_count) {
    bs_switchShader(curr_batch->shader->id);
    bs_renderBatchNoShader(start_index, draw_count);
}

void bs_clearBatch() {
    curr_batch->vertex_draw_count = 0;
    curr_batch->index_draw_count = 0;
}

int bs_batchSize() {
    return curr_batch->index_draw_count;
}

/* --- FRAMEBUFFERS --- */
void bs_framebuffer(bs_Framebuffer *framebuffer, bs_ivec2 dim) {
    curr_framebuffer = framebuffer;

    framebuffer->render_width  = dim.x;
    framebuffer->render_height = dim.y;
    framebuffer->clear = GL_DEPTH_BUFFER_BIT;

    glGenFramebuffers(1, &framebuffer->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->FBO);
}

void bs_attachColorbuffer(bs_Tex2D *color_buffer, int attachment) {
    bs_Framebuffer *framebuffer = curr_framebuffer;
    #ifdef BS_DEBUG
        if(framebuffer == NULL) {
            bs_print(BS_ERR, "COLORBUFFER ATTACH FAILED : Framebuffer is NULL");
        }
        if(color_buffer == NULL) {
            bs_print(BS_ERR, "COLORBUFFER ATTACH FAILED : Texture is NULL");
        }
        return;
    #endif

    framebuffer->clear |= GL_COLOR_BUFFER_BIT;

    // Attach it to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, GL_TEXTURE_2D, color_buffer->id, 0);
}

void bs_attachRenderbuffer() {
    bs_Framebuffer *framebuffer = curr_framebuffer;
    #ifdef BS_DEBUG
        if(framebuffer == NULL) {
            bs_print(BS_ERR, "RENDERBUFFER ATTACH FAILED : Framebuffer is NULL");
        }
        return;
    #endif

    glGenRenderbuffers(1, &framebuffer->RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebuffer->render_width, framebuffer->render_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebuffer->RBO); 
}

void bs_attachDepthBuffer(bs_Tex2D *tex) {
    bs_Framebuffer *framebuffer = curr_framebuffer;

    #ifdef BS_DEBUG
        if(framebuffer == NULL) {
            bs_print(BS_ERR, "DEPTHBUFFER ATTACH FAILED : Framebuffer is NULL");
        }
        if(tex == NULL) {
            bs_print(BS_ERR, "DEPTHBUFFER ATTACH FAILED : Texture is NULL");
        }
        return;
    #endif

    if(tex->type == BS_CUBEMAP) {
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex->id, 0);
	return;
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex->id, 0);
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

void bs_noDrawBuf() {
    glDrawBuffer(GL_NONE);
}

void bs_noReadBuf() {
    glDrawBuffer(GL_NONE);
}

void bs_startFramebufferRender(bs_Framebuffer *framebuffer) {
    glViewport(0, 0, framebuffer->render_width, framebuffer->render_height);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->FBO);

    glEnable(GL_DEPTH_TEST);

    // Clear any previous drawing
   // glClearColor(1.0, 0.0, 1.0, 0.0);
    glClear(framebuffer->clear);
}

void bs_endFramebufferRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bs_ivec2 res = bs_resolution();
    glViewport(0, 0, res.x, res.y);
}

// TODO: Remove this
typedef struct {
    float elapsed;
    bs_ivec2 res;
} bs_Globals;

void bs_setGlobalVars() {
    bs_Globals globals;

    globals.elapsed = bs_elapsedTime();
    globals.res = bs_resolution();

    bs_setUniformBlockData(global_unifs, &globals);
}

void bs_init(int width, int height, char *title) {
    bs_initWnd(width, height, title);
    // bs_printHardwareInfo();

    def_camera.pos.x = 0.0;
    def_camera.pos.y = 0.0;
    def_camera.pos.z = 500.0;
    bs_lookat(def_camera.view, def_camera.pos, (bs_vec3){ 0.0, 0.0, -1.0 }, (bs_vec3){ 0.0, 1.0, 0.0 });
    bs_ortho(def_camera.proj, 0, width, 0, height, 0.01, 1000.0);

    global_unifs = bs_initUniformBlock(sizeof(bs_Globals), 0);

    // Load default shaders
    bs_loadShader("resources/bs_texture_shader.vs", "resources/bs_texture_shader.fs", 0, &texture_shader);
}

void bs_startRender(void (*render)()) {
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    srand(time(0));

    bs_wndTick(render);
}
