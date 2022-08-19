// GL
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
#include <string.h>
#include <stddef.h>
#include <lodepng.h>

#ifdef _WIN32
    #include <windows.h>
    #include <objidl.h>
#endif

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
void bs_setOrthographicProjection(bs_Camera *cam, int left, int right, int bottom, int top, float nearZ, float farZ) {
    glm_ortho(left, right, bottom, top, nearZ, farZ, cam->proj);

    int x_res = bs_sign(left - right);
    int y_res = bs_sign(top - bottom);
}

void bs_setMatrixLookat(bs_Camera *cam, bs_vec3 center, bs_vec3 up) {
    glm_lookat((vec3){ cam->pos.x, cam->pos.y, cam->pos.z }, (vec3){ center.x, center.y, center.z }, (vec3){ up.x, up.y, up.z }, cam->view);
}

void bs_setMatrixLook(bs_Camera *cam, bs_vec3 dir, bs_vec3 up) {
    glm_look((vec3){ cam->pos.x, cam->pos.y, cam->pos.z }, (vec3){ dir.x, dir.y, dir.z }, (vec3){ up.x, up.y, up.z }, cam->view);
}

void bs_setPerspectiveProjection(bs_Camera *cam, float aspect, float fovy, float nearZ, float farZ) {
    glm_perspective(glm_rad(fovy), aspect, nearZ, farZ, cam->proj);
}

/* --- UNBATCHED RENDERING --- */

/* --- BATCHED RENDERING --- */
void bs_selectBatch(bs_Batch *batch) {
    curr_batch = batch;

    glBindVertexArray(batch->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, batch->VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->EBO);

    bs_switchShader(curr_batch->shader->id);
}

void bs_pushVertexStruct(void *vertex) {
    bs_Batch *batch = curr_batch;
    memcpy(curr_batch->vertices + curr_batch->vertex_draw_count * batch->attrib_size_bytes, vertex, batch->attrib_size_bytes);
    curr_batch->vertex_draw_count++;
}

void bs_pushVertex(
    bs_vec3 pos, 
    bs_vec2 tex_coord, 
    bs_vec3 normal, 
    bs_RGBA color, 
    bs_vec4 attrib_vec4
) {
    bs_Batch *batch = curr_batch;

    int offset = 0;
    unsigned char data[curr_batch->attrib_size_bytes];
    bool has_position  = (batch->shader->attribs & BS_POSITION) == BS_POSITION;
    bool has_normal    = (batch->shader->attribs & BS_NORMAL) == BS_NORMAL;
    bool has_tex_coord = (batch->shader->attribs & BS_TEX_COORD) == BS_TEX_COORD;
    bool has_color     = (batch->shader->attribs & BS_COLOR) == BS_COLOR;
    bool has_attr_vec4 = (batch->shader->attribs & BS_ATTR_VEC4) == BS_ATTR_VEC4;

    memcpy(data + offset, &pos, sizeof(bs_vec3) * has_position);
    offset += sizeof(bs_vec3) * has_position;

    memcpy(data + offset, &tex_coord, sizeof(bs_vec2) * has_tex_coord);
    offset += sizeof(bs_vec2) * has_tex_coord;

    memcpy(data + offset, &color, sizeof(bs_RGBA) * has_color);
    offset += sizeof(bs_RGBA) * has_color;

    memcpy(data + offset, &normal, sizeof(bs_vec3) * has_normal); 
    offset += sizeof(bs_vec3) * has_normal;

    memcpy(data + offset, &attrib_vec4, sizeof(bs_vec4) * has_attr_vec4); 
    offset += sizeof(bs_vec4) * has_attr_vec4;

    bs_pushVertexStruct(data);
}

// void bs_pushTesselatedRect(, int polys_per_side) {
// }

void bs_pushAtlasSlice(bs_vec3 pos, bs_vec2 dim, bs_RGBA col, bs_Slice *slice) {
    bs_vec2 dim_pos = { dim.x + pos.x, dim.y + pos.y };

    int indices[] = {
        curr_batch->vertex_draw_count+0, curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2,
        curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2, curr_batch->vertex_draw_count+3,
    };
    memcpy(&curr_batch->indices[curr_batch->index_draw_count], indices, 6 * sizeof(int));

    bs_pushVertex((bs_vec3){ pos.x    , pos.y    , pos.z }, (bs_vec2){ slice->tex_x , slice->tex_hy }, (bs_vec3){ 0.0, 0.0, 0.0 }, col, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 }); // Bottom Left
    bs_pushVertex((bs_vec3){ dim_pos.x, pos.y    , pos.z }, (bs_vec2){ slice->tex_wx, slice->tex_hy }, (bs_vec3){ 0.0, 0.0, 0.0 }, col, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 }); // Bottom right
    bs_pushVertex((bs_vec3){ pos.x    , dim_pos.y, pos.z }, (bs_vec2){ slice->tex_x , slice->tex_y  }, (bs_vec3){ 0.0, 0.0, 0.0 }, col, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 }); // Top Left
    bs_pushVertex((bs_vec3){ dim_pos.x, dim_pos.y, pos.z }, (bs_vec2){ slice->tex_wx, slice->tex_y  }, (bs_vec3){ 0.0, 0.0, 0.0 }, col, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 }); // Top Right

    curr_batch->index_draw_count += 6;
}

void bs_pushTex2D(bs_vec3 pos, bs_vec2 dim, bs_RGBA col) {
    bs_vec2 dim_pos = { dim.x + pos.x, dim.y + pos.y };

    int indices[] = {
        curr_batch->vertex_draw_count+0, curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2,
        curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2, curr_batch->vertex_draw_count+3,
    };

    memcpy(&curr_batch->indices[curr_batch->index_draw_count], indices, 6 * sizeof(int));

    bs_pushVertex((bs_vec3){ pos.x    , pos.y    , pos.z }, (bs_vec2){ 0.0, 0.0 }, (bs_vec3){ 0.0, 0.0, 0.0 }, col, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 }); // Bottom Left
    bs_pushVertex((bs_vec3){ dim_pos.x, pos.y    , pos.z }, (bs_vec2){ 1.0, 0.0 }, (bs_vec3){ 0.0, 0.0, 0.0 }, col, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 }); // Bottom right
    bs_pushVertex((bs_vec3){ pos.x    , dim_pos.y, pos.z }, (bs_vec2){ 0.0, 1.0 }, (bs_vec3){ 0.0, 0.0, 0.0 }, col, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 }); // Top Left
    bs_pushVertex((bs_vec3){ dim_pos.x, dim_pos.y, pos.z }, (bs_vec2){ 1.0, 1.0 }, (bs_vec3){ 0.0, 0.0, 0.0 }, col, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 }); // Top Right

    curr_batch->index_draw_count += 6;
}

void bs_pushRect(bs_vec3 pos, bs_vec2 dim, bs_RGBA col) {
    bs_vec2 dim_pos = { dim.x + pos.x, dim.y + pos.y };

    int indices[] = {
        curr_batch->vertex_draw_count+0, curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2,
        curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2, curr_batch->vertex_draw_count+3,
    };

    memcpy(&curr_batch->indices[curr_batch->index_draw_count], indices, 6 * sizeof(int));

    const float white_tex_coord = 0.9999;
    bs_pushVertex((bs_vec3){ pos.x    , pos.y    , pos.z }, (bs_vec2){ white_tex_coord, white_tex_coord }, (bs_vec3){ 0.0, 0.0, 0.0 }, col, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 }); // Bottom Left
    bs_pushVertex((bs_vec3){ dim_pos.x, pos.y    , pos.z }, (bs_vec2){ white_tex_coord, white_tex_coord }, (bs_vec3){ 0.0, 0.0, 0.0 }, col, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 }); // Bottom right
    bs_pushVertex((bs_vec3){ pos.x    , dim_pos.y, pos.z }, (bs_vec2){ white_tex_coord, white_tex_coord }, (bs_vec3){ 0.0, 0.0, 0.0 }, col, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 }); // Top Left
    bs_pushVertex((bs_vec3){ dim_pos.x, dim_pos.y, pos.z }, (bs_vec2){ white_tex_coord, white_tex_coord }, (bs_vec3){ 0.0, 0.0, 0.0 }, col, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 }); // Top Right

    curr_batch->index_draw_count += 6;
}

void bs_pushTriangle(bs_vec3 pos1, bs_vec3 pos2, bs_vec3 pos3, bs_RGBA color) {
    int indices[] = {
        curr_batch->vertex_draw_count+0, curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2,
    };
    memcpy(&curr_batch->indices[curr_batch->index_draw_count], indices, 3 * sizeof(int));

    bs_pushVertex(pos1, (bs_vec2){ 0.0, 0.0 }, (bs_vec3){ 0.0, 0.0, 0.0 }, color, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 });
    bs_pushVertex(pos2, (bs_vec2){ 1.0, 0.0 }, (bs_vec3){ 0.0, 0.0, 0.0 }, color, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 });
    bs_pushVertex(pos3, (bs_vec2){ 0.0, 1.0 }, (bs_vec3){ 0.0, 0.0, 0.0 }, color, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 });

    curr_batch->index_draw_count += 3;
}

void bs_pushLine(bs_vec3 start, bs_vec3 end, bs_RGBA color) {
    bs_pushTriangle(start, end, end, color);
}

void bs_pushPrim(bs_Prim *prim, bs_Mesh *mesh) {
    for(int i = 0; i < prim->index_count; i++) {
        curr_batch->indices[curr_batch->index_draw_count+i] = prim->indices[i] + curr_batch->vertex_draw_count;
    }

    for(int i = 0; i < prim->vertex_count; i++) {
        const float white_tex_coord = 0.9999;
        bs_vec2 tex_coord = (bs_vec2){ white_tex_coord, white_tex_coord };

        if(prim->material.tex != NULL) {
            // TODO: These values are constant, unnecessary to set them every frame
            tex_coord.x = prim->vertices[i].tex_coord.x;/* + prim->material.tex->tex_x;*/
            tex_coord.y = prim->vertices[i].tex_coord.y;/* + prim->material.tex->tex_y;*/
        }

        bs_pushVertex(prim->vertices[i].position, tex_coord, prim->vertices[i].normal, prim->material.base_color, (bs_vec4){ 0.0, 0.0, 0.0, 0.0 });

        // bs_pushVertexStruct(&vertex);
    }

    curr_batch->index_draw_count += prim->index_count;
}

void bs_pushMesh(bs_Mesh *mesh) {
    for(int i = 0; i < mesh->prim_count; i++) {
        bs_Prim *prim = &mesh->prims[i];
        bs_pushPrim(prim, mesh);
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

void bs_createBatch(bs_Batch *batch, bs_Shader *shader, int index_count) {
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
            bs_addBatchAttrib(data->type, data->count, data->size, data->normalized);
        }
    }

    // Allocate buffer spaces
    batch->indices = malloc(sizeof(int) * index_count);
    batch->vertices = malloc(batch->attrib_size_bytes * index_count);
    glBufferData(GL_ARRAY_BUFFER, batch->attrib_size_bytes * index_count, NULL, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * index_count, NULL, GL_STATIC_DRAW);
}

void bs_setBatchRawData(void *vertex_data, void *index_data, int vertex_size, int index_size) {
    glBufferData(GL_ARRAY_BUFFER, vertex_size, vertex_data, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_size, index_data, GL_STATIC_DRAW);
}

void bs_addBatchAttribI(const int type, unsigned int amount, size_t size_per_type) {
    bs_Batch *batch = curr_batch;

    glEnableVertexAttribArray(batch->attrib_count);
    glVertexAttribIPointer(batch->attrib_count++, amount, type, batch->attrib_size_bytes, (void*)batch->attrib_offset);

    batch->attrib_offset += size_per_type;
}

void bs_addBatchAttrib(const int type, unsigned int amount, size_t size_per_type, bool normalized) {
    if((type >= BS_SHORT) && (type <= BS_INT)) {
        bs_addBatchAttribI(type, amount, size_per_type);
        return;
    }

    bs_Batch *batch = curr_batch;

    glEnableVertexAttribArray(batch->attrib_count);
    glVertexAttribPointer(batch->attrib_count++, amount, type, normalized, batch->attrib_size_bytes, (void*)batch->attrib_offset);

    batch->attrib_offset += size_per_type;
}

void bs_bindBufferRange(int target, int bind_point, int buffer, int offset, int size) {
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

void bs_renderBatch(int start_index, int draw_count) {
    // Batch should still be bound here
    // bs_setTimeUniform(curr_batch->shader, 0.0);

    #ifdef BS_DEBUG
        // bs_Camera *cam = curr_batch->camera;
        // if(bs_debugCameraIsActivated()) {
            // cam = bs_getDebugCamera();
        // }
        // bs_setViewMatrixUniform(curr_batch->shader, cam);
        // bs_setProjMatrixUniform(curr_batch->shader, cam);
        // bs_setViewMatrixUniform(curr_batch->shader, curr_batch->camera);
        // bs_setProjMatrixUniform(curr_batch->shader, curr_batch->camera);
    #else 
        bs_setViewMatrixUniform(curr_batch->shader, curr_batch->camera);
        bs_setProjMatrixUniform(curr_batch->shader, curr_batch->camera);
    #endif

    glDrawElements(curr_batch->draw_mode, draw_count, GL_UNSIGNED_INT, (void*)(start_index * 6 * sizeof(GLuint)));
}

void bs_clearBatch() {
    curr_batch->vertex_draw_count = 0;
    curr_batch->index_draw_count = 0;
}

// TODO: Gör current_batch
int bs_getBatchSize(bs_Batch *batch) {
    return batch->index_draw_count;
}

/* --- FRAMEBUFFERS --- */
void bs_createFramebuffer(bs_Framebuffer *framebuffer, int render_width, int render_height) {
    curr_framebuffer = framebuffer;

    framebuffer->render_width  = render_width;
    framebuffer->render_height = render_height;
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

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex->id, 0);
}

void bs_setDrawBufs(int n, ...) {
    int values[n];

    va_list ptr;
    va_start(ptr, n);
    for(int i = 0; i < n; i++) {
        values[i] = GL_COLOR_ATTACHMENT0 + va_arg(ptr, int);
    }
    va_end(ptr);

    glDrawBuffers(n, values);
}

void bs_startFramebufferRender(bs_Framebuffer *framebuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->FBO);

    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Clear any previous drawing
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(framebuffer->clear);
}

void bs_endFramebufferRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
}

typedef struct {
    float elapsed;
    bs_ivec2 res;
} bs_Globals;

void bs_setGlobalVars() {
    bs_Globals globals;

    globals.elapsed  = bs_elapsedTimef();
    globals.res      = bs_resolution();

    bs_setUniformBlockData(global_unifs, &globals);
}

void bs_init(int width, int height, char *title) {
    bs_initWnd(width, height, title);
    // bs_printHardwareInfo();

    def_camera.pos.x = 0.0;
    def_camera.pos.y = 0.0;
    def_camera.pos.z = 500.0;
    bs_setMatrixLookat(&def_camera, (bs_vec3){ 0.0, 0.0, -1.0 }, (bs_vec3){ 0.0, 1.0, 0.0 });
    bs_setOrthographicProjection(&def_camera, 0, width, 0, height, 0.01, 1000.0);

    global_unifs = bs_initUniformBlock(sizeof(bs_Globals), 0);

    // Texture Atlas Init
    // std_atlas = bs_createTextureAtlas(BS_ATLAS_SIZE, BS_ATLAS_SIZE, BS_MAX_TEXTURES);

    // Load default shaders
    bs_loadShader("resources/bs_texture_shader.vs", "resources/bs_texture_shader.fs", 0, &texture_shader);
}

void bs_startRender(void (*render)()) {
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bs_wndTick(render);
}