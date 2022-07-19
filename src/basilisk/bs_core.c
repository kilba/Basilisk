// GL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

// Basilisk
#include <bs_shaders.h>
#include <bs_textures.h>
#include <bs_debug.h>
#include <bs_core.h>
#include <bs_math.h>

// STD
#include <string.h>
#include <stddef.h>
#include <lodepng.h>

#ifdef _WIN32
    #include <windows.h>
    #include <objidl.h>
#endif

float screen_quad_vertices[] = {
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

struct bs_Window {
    int width;
    int height;

    // Pointers to all framebuffers
    bs_Framebuffer **framebuffers;
    int allocated_framebuffer_count;
    int framebuffer_count;
} bs_window;

GLFWwindow *window;

bs_Framebuffer std_framebuffer;

// Shaders
bs_Shader fbo_shader;
bs_Shader model_shader;
bs_Shader texture_shader;

bs_Camera std_camera;
bs_Batch *curr_batch;
bs_Tex2D empty_texture;

bs_Atlas *std_atlas;

float elapsed_time = 0.0;
bs_fRGBA clear_color = { 0.0, 0.0, 0.0, 1.0 };

/* --- WINDOW SETTINGS --- */
void bs_initGLFW(bs_WNDSettings settings) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if(settings != bs_WND_DEFAULT) {
        if((settings & bs_WND_TRANSPARENT) == bs_WND_TRANSPARENT) {
            glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);
        }
        if((settings & bs_WND_NO_TITLE_BAR) == bs_WND_NO_TITLE_BAR) {
            glfwWindowHint(GLFW_DECORATED, false);
        }
        if((settings & bs_WND_TOPMOST) == bs_WND_TOPMOST) {
            glfwWindowHint(GLFW_FLOATING, true);
        }
        if((settings & bs_WND_INVISIBLE) == bs_WND_INVISIBLE) {
            glfwWindowHint(GLFW_VISIBLE, false);
        }
        if((settings & bs_WND_UNCLICKABLE) == bs_WND_UNCLICKABLE) {
            glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, true);
        }
    }
}

void bs_createWindow(int width, int height, char* title) {
    bs_window.width = width;
    bs_window.height = height;

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGL();
    glViewport(0, 0, width, height);
}

// TODO: Extract to bs_debug.c
void bs_printHardwareInfo() {
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    bs_print(BS_CLE, "%s\n", vendor);
    bs_print(BS_CLE, "%s\n", renderer);
}

void bs_setBackgroundColor(bs_fRGBA color) {
    clear_color = color;
    clear_color.r /= 255.0;
    clear_color.g /= 255.0;
    clear_color.b /= 255.0;
    clear_color.a /= 255.0;
}

bs_vec2 bs_getWindowDimensions() {
    return (bs_vec2){ bs_window.width, bs_window.height };
}

#ifdef _WIN32
    BITMAPINFOHEADER createBitmapHeader(int width, int height) {
        BITMAPINFOHEADER  bi;

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

        DWORD dwBmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;
        HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
        unsigned char* lpbitmap = (unsigned char*)GlobalLock(hDIB);

        // copy from the window device context to the bitmap device context
        StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screenx, screeny, width, height, SRCCOPY);   //change SRCCOPY to NOTSRCCOPY for wacky colors !
        GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

        GlobalFree(hDIB);
        DeleteDC(hwindowCompatibleDC);
        ReleaseDC(hWnd, hwindowDC);

        return lpbitmap;
    }
#endif

/* --- INPUTS/CALLBACKS --- */
bool key_states[350];

void bs_onKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action == GLFW_PRESS) {
        key_states[key] = true;
        return;
    }
    if (action == GLFW_RELEASE) {
        key_states[key] = false;
        return;
    }
}

bool bs_isKeyDown(int key) {
    return key_states[key];
}

bool bs_isKeyUp(int key) {
    return !key_states[key];
}

bool bs_isKeyDownOnce(int key) {
    bool result = key_states[key] != false;
    key_states[key] = false;

    return result;
}

bool bs_isKeyUpOnce(int key) {
    bool result = key_states[key] != true;
    key_states[key] = true;

    return result;
}

void bs_onResize(GLFWwindow* window, int width, int height) {
    // glfwSetWindowSize(window, width, height);
    // glViewport(0, 0, width, height);
    // bs_window.width  = width;
    // bs_window.height = height;
}

bs_vec2 bs_getCursorPositionReverseY() {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    return (bs_vec2){ xpos, ypos };
}

bs_vec2 bs_getCursorPosition() {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    return (bs_vec2){ xpos, bs_window.height - ypos };
}

/* --- MATRICES --- */
void bs_setProjMatrixOrtho(bs_Camera *cam, int left, int right, int bottom, int top) {
    glm_ortho(left, right, bottom, top, 0.1, 10000.0, cam->proj);
    // glm_translate(cam->view, (vec3){ 0.0, 0.0, -1.0 });
}

void bs_setViewMatrixOrtho(bs_Camera *cam) {
    float cpy_matrix[4][4] = GLM_MAT4_IDENTITY_INIT;
    memcpy(cam->view, cpy_matrix, 4 * 4 * sizeof(float));
}

void bs_createOrthographicProjection(bs_Camera *cam, int left, int right, int bottom, int top) {
    // bs_setViewMatrixOrtho(cam);
    bs_setProjMatrixOrtho(cam, left, right, bottom, top);

    int x_res = bs_sign(left - right);
    int y_res = bs_sign(top - bottom);

    cam->res = (bs_vec2){ x_res, y_res };
}

void bs_setMatrixLookat(bs_Camera *cam, bs_vec3 center, bs_vec3 up) {
    glm_lookat((vec3){ cam->pos.x, cam->pos.y, cam->pos.z }, (vec3){ center.x, center.y, center.z }, (vec3){ up.x, up.y, up.z }, cam->view);
}

void bs_setMatrixLook(bs_Camera *cam, bs_vec3 dir, bs_vec3 up) {
    glm_look((vec3){ cam->pos.x, cam->pos.y, cam->pos.z }, (vec3){ dir.x, dir.y, dir.z }, (vec3){ up.x, up.y, up.z }, cam->view);
}

void bs_setPerspectiveProjection(bs_Camera *cam, bs_vec2 res, float fovy, float nearZ, float farZ) {
    glm_perspective(glm_rad(fovy), res.x / res.y, nearZ, farZ, cam->proj);
    cam->res = res;
}

bs_Camera *bs_getStdCamera() {
    return &std_camera;
}

/* --- BATCHING --- */
void bs_selectBatch(bs_Batch *batch) {
    curr_batch = batch;

    glBindVertexArray(batch->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, batch->VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->EBO);

    bs_switchShader(curr_batch->shader);
}

bs_Atlas *bs_getStdAtlas() {
    return std_atlas;
}

void bs_pushVertexStruct(void *vertex) {
    bs_Batch *batch = curr_batch;
    memcpy(curr_batch->vertices + curr_batch->vertex_draw_count * batch->attrib_size_bytes, vertex, batch->attrib_size_bytes);
    curr_batch->vertex_draw_count++;
}

void bs_pushVertex(float px, float py, float pz, float tx, float ty, float nx, float ny, float nz, bs_RGBA color) {
    bs_Vertex push_vertex;

    push_vertex.position.x = px;
    push_vertex.position.y = py;
    push_vertex.position.z = pz;

    push_vertex.tex_coord.x = tx;
    push_vertex.tex_coord.y = ty;

    // TODO: Make these optional
    push_vertex.normal.x = nx;
    push_vertex.normal.y = ny;
    push_vertex.normal.z = nz;

    push_vertex.color = color;

    bs_pushVertexStruct(&push_vertex);
}

void bs_pushTexRect(bs_vec3 pos, bs_vec2 dim, bs_RGBA col, bs_Tex2D *tex) {
    bs_vec2 dim_pos = { dim.x + pos.x, dim.y + pos.y };

    int indices[] = {
        curr_batch->vertex_draw_count+0, curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2,
        curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2, curr_batch->vertex_draw_count+3,
    };
    memcpy(&curr_batch->indices[curr_batch->index_draw_count], indices, 6 * sizeof(int));

    bs_pushVertex(pos.x    , pos.y    , pos.z, tex->tex_x , tex->tex_hy, 0.0, 0.0, 0.0, col); // Bottom Left
    bs_pushVertex(dim_pos.x, pos.y    , pos.z, tex->tex_wx, tex->tex_hy, 0.0, 0.0, 0.0, col); // Bottom right
    bs_pushVertex(pos.x    , dim_pos.y, pos.z, tex->tex_x , tex->tex_y , 0.0, 0.0, 0.0, col); // Top Left
    bs_pushVertex(dim_pos.x, dim_pos.y, pos.z, tex->tex_wx, tex->tex_y , 0.0, 0.0, 0.0, col); // Top Right

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
    bs_pushVertex(pos.x    , pos.y    , pos.z, white_tex_coord, white_tex_coord, 0.0, 0.0, 0.0, col); // Bottom Left
    bs_pushVertex(dim_pos.x, pos.y    , pos.z, white_tex_coord, white_tex_coord, 0.0, 0.0, 0.0, col); // Bottom right
    bs_pushVertex(pos.x    , dim_pos.y, pos.z, white_tex_coord, white_tex_coord, 0.0, 0.0, 0.0, col); // Top Left
    bs_pushVertex(dim_pos.x, dim_pos.y, pos.z, white_tex_coord, white_tex_coord, 0.0, 0.0, 0.0, col); // Top Right

    curr_batch->index_draw_count += 6;
}

void bs_pushTriangle(bs_vec3 pos1, bs_vec3 pos2, bs_vec3 pos3, bs_RGBA color) {
    int indices[] = {
        curr_batch->vertex_draw_count+0, curr_batch->vertex_draw_count+1, curr_batch->vertex_draw_count+2,
    };
    memcpy(&curr_batch->indices[curr_batch->index_draw_count], indices, 3 * sizeof(int));

    bs_pushVertex(pos1.x, pos1.y, pos1.z, 0.0, 0.0, 0.0, 0.0, 0.0, color);
    bs_pushVertex(pos2.x, pos2.y, pos2.z, 1.0, 0.0, 0.0, 0.0, 0.0, color);
    bs_pushVertex(pos3.x, pos3.y, pos3.z, 0.0, 1.0, 0.0, 0.0, 0.0, color);

    curr_batch->index_draw_count += 3;
}

void bs_pushLine(bs_vec3 start, bs_vec3 end, bs_RGBA color) {
    bs_pushTriangle(start, end, end, color);
}

void bs_pushPrim(bs_Prim *prim, mat4 model, bs_Mesh *mesh) {
    for(int i = 0; i < prim->index_count; i++) {
        curr_batch->indices[curr_batch->index_draw_count+i] = prim->indices[i] + curr_batch->vertex_draw_count;
    }

    for(int i = 0; i < prim->vertex_count; i++) {
        bs_RVertex vertex;
        void *v_vertex = &vertex;

        memcpy(v_vertex + offsetof(bs_Vertex, color)   , &prim->material.base_color, sizeof(bs_RGBA));
        memcpy(v_vertex + offsetof(bs_Vertex, normal)  , &prim->vertices[i].normal, sizeof(bs_vec3));
        memcpy(v_vertex + offsetof(bs_Vertex, position), &prim->vertices[i].position, sizeof(bs_vec3));

        vertex.bone_ids = prim->vertices[i].bone_ids;
        vertex.weights  = prim->vertices[i].weights;

        // TODO: Figure out why 1.0 causes glitchy rendering
        const float white_tex_coord = 0.9999;
        vertex.tex_coord = (bs_vec2){ white_tex_coord, white_tex_coord };

        if(prim->material.tex != NULL) {
            // TODO: These values are constant, unnecessary to set them every frame
            vertex.tex_coord.x = prim->vertices[i].tex_coord.x + prim->material.tex->tex_x;
            vertex.tex_coord.y = prim->vertices[i].tex_coord.y + prim->material.tex->tex_y;
        }

        bs_pushVertexStruct(&vertex);
    }

    curr_batch->index_draw_count += prim->index_count;
}

void bs_pushMesh(bs_Mesh *mesh) {
    mat4 model = GLM_MAT4_IDENTITY_INIT;

    vec3 glm_pos = { mesh->pos.x, mesh->pos.y, mesh->pos.z };
    vec3 glm_sca = { mesh->sca.x, mesh->sca.y, mesh->sca.z };
    versor glm_rot = { mesh->rot.x, mesh->rot.y, mesh->rot.z, mesh->rot.w }; 

    glm_translate(model, glm_pos);
    glm_quat_rotate(model, glm_rot, model);
    glm_scale(model, glm_sca);

    for(int i = 0; i < mesh->prim_count; i++) {
        bs_Prim *prim = &mesh->prims[i];
        bs_pushPrim(prim, model, mesh);
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

void bs_createBatch(bs_Batch *batch, int index_count, const int batch_type, int batch_size_bytes) {
    // Default values
    batch->camera = &std_camera;
    batch->shader = &texture_shader;
    batch->draw_mode = BS_TRIANGLES;
    batch->vertex_draw_count = 0;
    batch->index_draw_count = 0;
    batch->attrib_count = 0;
    batch->attrib_size_bytes = batch_size_bytes;

    // Create buffer/array objects
    glGenVertexArrays(1, &batch->VAO);
    glGenBuffers(1, &batch->VBO);
    glGenBuffers(1, &batch->EBO);

    bs_selectBatch(batch);

    // Attribute setup
    bs_addBatchAttrib (BS_FLOAT, 3, offsetof(bs_Vertex, position) , false);
    bs_addBatchAttrib (BS_FLOAT, 2, offsetof(bs_Vertex, tex_coord), false);
    bs_addBatchAttrib (BS_FLOAT, 3, offsetof(bs_Vertex, normal)   , false);
    bs_addBatchAttrib (BS_UBYTE, 4, offsetof(bs_Vertex, color)    , true);
    if(batch_type == BS_RIG_BATCH) {
        bs_addBatchAttribI(GL_INT  , 4, offsetof(bs_RVertex, bone_ids));
        bs_addBatchAttrib (BS_FLOAT, 4, offsetof(bs_RVertex, weights)  , false);
        batch->shader = &model_shader;
    }

    batch->indices = malloc(sizeof(int) * index_count);
    batch->vertices = malloc(batch->attrib_size_bytes * index_count);
    glBufferData(GL_ARRAY_BUFFER, batch->attrib_size_bytes * index_count, NULL, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * index_count, NULL, GL_STATIC_DRAW);
}

void bs_addBatchAttrib(const int type, unsigned int amount, size_t offset_bytes, bool normalized) {
    bs_Batch *batch = curr_batch;

    glEnableVertexAttribArray(batch->attrib_count);
    glVertexAttribPointer(batch->attrib_count++, amount, type, normalized, batch->attrib_size_bytes, (void*)offset_bytes);
}

void bs_addBatchAttribI(const int type, unsigned int amount, size_t offset_bytes) {
    bs_Batch *batch = curr_batch;

    glEnableVertexAttribArray(batch->attrib_count);
    glVertexAttribIPointer(batch->attrib_count++, amount, type, batch->attrib_size_bytes, (void*)offset_bytes);
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
    bs_setTimeUniform(curr_batch->shader, elapsed_time);
    bs_setViewMatrixUniform(curr_batch->shader, curr_batch->camera);
    bs_setProjMatrixUniform(curr_batch->shader, curr_batch->camera);

    glDrawElements(curr_batch->draw_mode, draw_count, GL_UNSIGNED_INT, (void*)(start_index * 6 * sizeof(GLuint)));
}

void bs_clearBatch() {
    curr_batch->vertex_draw_count = 0;
    curr_batch->index_draw_count = 0;
}

int bs_getBatchSize(bs_Batch *batch) {
    return batch->index_draw_count;
}

/* --- FRAMEBUFFERS --- */
void bs_attachColorbuffer(bs_Framebuffer *framebuffer) {
    glGenTextures(1, &framebuffer->texture_color_buffer);
    glBindTexture(GL_TEXTURE_2D, framebuffer->texture_color_buffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer->render_width, framebuffer->render_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach it to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer->texture_color_buffer, 0);
}

void bs_attachRenderbuffer(bs_Framebuffer *framebuffer) {
    glGenRenderbuffers(1, &framebuffer->RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebuffer->render_width, framebuffer->render_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebuffer->RBO); 
}

void bs_setFramebufferVertices(bs_Framebuffer *framebuffer) {
    glGenVertexArrays(1, &framebuffer->VAO);
    glGenBuffers(1, &framebuffer->VBO);

    glBindVertexArray(framebuffer->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, framebuffer->VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(screen_quad_vertices), &screen_quad_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void bs_createFramebuffer(bs_Framebuffer *framebuffer, int render_width, int render_height, void (*render)(), bs_Shader *shader) {
    framebuffer->render_width  = render_width;
    framebuffer->render_height = render_height;
    framebuffer->render = render;

    // Allocate 4 more framebuffers if the framebuffer count goes out of range
    if((bs_window.framebuffer_count + 1) > bs_window.allocated_framebuffer_count) {
        bs_window.allocated_framebuffer_count += 4;
        bs_window.framebuffers = realloc(bs_window.framebuffers, bs_window.allocated_framebuffer_count * sizeof(bs_Framebuffer));
    }

    bs_window.framebuffers[bs_window.framebuffer_count++] = framebuffer;
    if(shader == NULL) {
        bs_setFramebufferShader(framebuffer, &fbo_shader);
    } else {
        bs_setFramebufferShader(framebuffer, shader);
    }

    glGenFramebuffers(1, &framebuffer->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->FBO);

    bs_attachColorbuffer(framebuffer);
    bs_attachRenderbuffer(framebuffer);
    bs_setFramebufferVertices(framebuffer);
}

void bs_setFramebufferShader(bs_Framebuffer *framebuffer, bs_Shader *shader) {
    framebuffer->shader = shader;
}

void bs_startFramebufferRender(bs_Framebuffer *framebuffer) {
    // Bind
    glBindVertexArray(framebuffer->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, framebuffer->VBO);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->FBO);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Clear any previous drawing
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void bs_endFramebufferRender(bs_Framebuffer *framebuffer) {
    bs_switchShader(framebuffer->shader);
    bs_setTimeUniform(framebuffer->shader, elapsed_time);

    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(framebuffer->VAO);

    // Render
    glBindTexture(GL_TEXTURE_2D, framebuffer->texture_color_buffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void bs_checkGLError() {
    GLenum err = glGetError();
    if(err != GL_NO_ERROR) {
        switch(err) {
            case GL_INVALID_ENUM                 : bs_print(BS_CLE, "INVALID_ENUM"); break;
            case GL_INVALID_VALUE                : bs_print(BS_CLE, "INVALID_VALUE"); break;
            case GL_INVALID_OPERATION            : bs_print(BS_CLE, "GL_INVALID_OPERATION"); break;
            case GL_STACK_OVERFLOW               : bs_print(BS_CLE, "GL_STACK_OVERFLOW"); break;
            case GL_STACK_UNDERFLOW              : bs_print(BS_CLE, "GL_STACK_UNDERFLOW"); break;
            case GL_OUT_OF_MEMORY                : bs_print(BS_CLE, "OUT_OF_MEMORY"); break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: bs_print(BS_CLE, "INVALID_FRAMEBUFFER_OPERATION"); break;
        }

        bs_print(BS_CLE, " | 0x0%x\n", err);
    }
}

void bs_render() {
    double previousTime = glfwGetTime();
    int frameCount = 0;

    for(int i = 0; i < 350; i++) {
        key_states[i] = false;
    }

    glfwSetKeyCallback(window, bs_onKey);
    glfwSetWindowSizeCallback(window, bs_onResize);

    while(!glfwWindowShouldClose(window)) {
        elapsed_time += 0.01;

        // Measure speed
        double currentTime = glfwGetTime();
        frameCount++;
        // If a second has passed.
        if (currentTime - previousTime >= 1.0) {
            // Display the frame count here any way you want.
            bs_print(BS_CLE, "%d\n", frameCount);

            frameCount = 0;
            previousTime = currentTime;
        }

        glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render all framebuffers
        for(int i = 0; i < bs_window.framebuffer_count; i++) {
            bs_Framebuffer *framebuffer = bs_window.framebuffers[i];
            bs_startFramebufferRender(framebuffer);
            bs_selectAtlas(std_atlas);
            framebuffer->render();
            bs_endFramebufferRender(framebuffer);
        }

        bs_checkGLError();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}

void bs_exitGLFW() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void bs_init(int width, int height, char *title, bs_WNDSettings settings) {
    bs_initGLFW(settings);

    // Set default texture to be empty
    empty_texture.w = empty_texture.h = 0;
    empty_texture.x = empty_texture.y = 0;
    empty_texture.tex_x = empty_texture.tex_y = 0;
    empty_texture.tex_wx = empty_texture.tex_hy = 0;
    empty_texture.data = NULL;
    bs_selectTexture(&empty_texture);

    // Allocate 4 framebuffers by default, this auto-increments
    bs_window.allocated_framebuffer_count = 4;
    bs_window.framebuffer_count = 0;
    bs_window.framebuffers = malloc(bs_window.allocated_framebuffer_count * sizeof(bs_Framebuffer*));

    bs_createWindow(width, height, title);
    bs_printHardwareInfo();

    std_camera.pos.x = 0.0;
    std_camera.pos.y = 0.0;
    std_camera.pos.z = 50.0;
    bs_setMatrixLookat(&std_camera, (bs_vec3){ 0.0, 0.0, 0.0 }, (bs_vec3){ 0.0, 1.0, 0.0 });
    bs_setPerspectiveProjection(&std_camera, (bs_vec2){ width, height }, 90.0, 0.1, 5000.0);

    // Texture Atlas Init
    std_atlas = bs_createTextureAtlas(BS_ATLAS_SIZE, BS_ATLAS_SIZE, BS_MAX_TEXTURES);

    // Create the default framebuffer
    bs_loadShader("resources/fbo_shader.vs", "resources/fbo_shader.fs", 0, &fbo_shader);

    // Load default shaders
    bs_loadShader("resources/bs_texture_shader.vs", "resources/bs_texture_shader.fs", 0, &texture_shader);
    bs_loadShader("resources/bs_model_shader.vs"  , "resources/bs_model_shader.fs"  , 0, &model_shader);
}

void bs_startRender(void (*render)()) {
    bs_pushAtlas(std_atlas);
    // bs_saveAtlasToFile(std_atlas, "test1.png");
    bs_freeAtlasData(std_atlas);

    bs_createFramebuffer(&std_framebuffer, bs_window.width, bs_window.height, render, &fbo_shader);

    bs_render();
    bs_exitGLFW();
}