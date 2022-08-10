#ifndef BS_CORE_H
#define BS_CORE_H

#include <bs_types.h>

/* --- RENDERING --- */

void bs_pushVertexStruct(void *vertex);
void bs_pushVertex(bs_vec3 pos, bs_vec2 tex_coord, bs_vec3 normal, bs_RGBA color);
void bs_pushAtlasSlice(bs_vec3 pos, bs_vec2 dim, bs_RGBA col, bs_Slice *tex);
void bs_pushTex2D(bs_vec3 pos, bs_vec2 dim, bs_RGBA col);
void bs_pushRect(bs_vec3 pos, bs_vec2 dim, bs_RGBA col);
void bs_pushTriangle(bs_vec3 pos1, bs_vec3 pos2, bs_vec3 pos3, bs_RGBA color);
void bs_pushLine(bs_vec3 start, bs_vec3 end, bs_RGBA color);
void bs_pushMesh(bs_Mesh *mesh);
void bs_pushModel(bs_Model *model);

void bs_pushModelUnbatched(bs_Model *model, bs_Shader *shader);

/* --- FRAMEBUFFERS --- */
void bs_createFramebuffer(bs_Framebuffer *framebuffer, int render_width, int render_height);
void bs_attachColorbuffer(bs_Tex2D *color_buffer, int attachment);
void bs_attachRenderbuffer();
void bs_attachDepthBuffer(bs_Tex2D *tex);
void bs_startFramebufferRender(bs_Framebuffer *framebuffer);
void bs_endFramebufferRender();

/* --- BATCHING --- */
void bs_createBatch(bs_Batch *batch, bs_Shader *shader, int index_count, int batch_size_bytes);
void bs_setBatchRawData(void *vertex_data, void *index_data, int vertex_size, int index_size);
void bs_addBatchAttrib (const int type, unsigned int amount, size_t size_per_type, bool normalized);
void bs_addBatchAttribI(const int type, unsigned int amount, size_t size_per_type);
void bs_bindBufferRange(int target, int bind_point, int buffer, int offset, int size);
void bs_selectBatch(bs_Batch *batch);
void bs_pushBatch();
void bs_renderBatch(int start_index, int draw_count);

void bs_freeBatchData();
void bs_clearBatch();
void bs_changeBatchBufferSize(bs_Batch *batch, int index_count);

void bs_setBatchShader(bs_Batch *batch, bs_Shader *shader);
int bs_getBatchSize(bs_Batch *batch);

/* --- WINDOW INITIALIZATION --- */
void bs_init(int width, int height, char *title, int settings);
void bs_startRender(void (*render)());
void bs_setBackgroundColor(bs_fRGBA color);
bs_vec2 bs_getWindowDimensions();
void bs_initWndSetting(int setting, bool val);

/* --- INPUTS / CALLBACKS --- */
bool bs_isKeyDown(int key);
bool bs_isKeyUp(int key);
bool bs_isKeyDownOnce(int key);
bool bs_isKeyUpOnce(int key);
bs_vec2 bs_getCursorPositionReverseY();
bs_vec2 bs_getCursorPosition();

/* --- MATRICES --- */
void bs_setMatrices(bs_Shader *shader);
bs_Camera *bs_getStdCamera();
void bs_setProjMatrixOrtho(bs_Camera *cam, int left, int right, int bottom, int top);
void bs_setViewMatrixOrtho(bs_Camera *cam);
void bs_setOrthographicProjection(bs_Camera *cam, int left, int right, int bottom, int top, float nearZ, float farZ);
void bs_setMatrixLookat(bs_Camera *cam, bs_vec3 center, bs_vec3 up);
void bs_setMatrixLook(bs_Camera *cam, bs_vec3 dir, bs_vec3 up);
void bs_setPerspectiveProjection(bs_Camera *cam, float aspect, float fovy, float nearZ, float farZ);

/* --- CONSTANTS --- */
/* OPENGL FILTERING SETTINGS */
#define BS_NEAREST 0x2600
#define BS_NEAREST_MIPMAP_LINEAR 0x2702
#define BS_NEAREST_MIPMAP_NEAREST 0x2700

#define BS_LINEAR 0x2601
#define BS_LINEAR_MIPMAP_LINEAR 0x2703
#define BS_LINEAR_MIPMAP_NEAREST 0x2701

/* BATCH ATTRIBUTE TYPES  */
#define BS_STD_BATCH 0
#define BS_RIG_BATCH 1

#define BS_POSITION 1
#define BS_TEX_COORD 2
#define BS_COLOR 4
#define BS_NORMAL 8
#define BS_BONE_IDS 16
#define BS_WEIGHTS 32

/* RENDER MODES */
#define BS_POINTS 0x0000
#define BS_LINES 0x0001
#define BS_LINE_LOOP 0x0002
#define BS_LINE_STRIP 0x0003
#define BS_TRIANGLES 0x0004
#define BS_TRIANGLE_STRIP 0x0005
#define BS_TRIANGLE_FAN 0x0006
#define BS_LINES_ADJACENCY 0x000A
#define BS_LINE_STRIP_ADJACENCY 0x000B
#define BS_TRIANGLES_ADJACENCY 0x000C
#define BS_TRIANGLE_STRIP_ADJACENCY 0x000D

/* DATATYPES */
#define BS_SBYTE 0x1400
#define BS_UBYTE 0x1401
#define BS_SHORT 0x1402
#define BS_USHORT 0x1403
#define BS_INT 0x1404
#define BS_UINT 0x1405
#define BS_FLOAT 0x1406

/* ATLAS SETTINGS */
#define BS_ATLAS_SIZE 4096 /* Pixels (x, y) */
#define BS_MAX_TEXTURES 1000

#define BS_FBO_ALLOC_BY 2

/* BASE INTERNAL FORMATS */
#define BS_CHANNEL_RED 0x1903
#define BS_CHANNEL_GREEN 0x1904
#define BS_CHANNEL_BLUE 0x1905
#define BS_CHANNEL_ALPHA 0x1906
#define BS_CHANNEL_RGB 0x1907
#define BS_CHANNEL_RGBA 0x1908

/* SIZED INTERNAL FORMATS */
#define BS_CHANNEL_RGBA32F 0x8814
#define BS_CHANNEL_RGB32F 0x8815
#define BS_CHANNEL_RGBA16F 0x881A
#define BS_CHANNEL_RGB16F 0x881B

/* DEPTH INTERNAL FORMATS */
#define BS_CHANNEL_DEPTH 0x1902
#define BS_CHANNEL_DEPTH_16 0x81A5
#define BS_CHANNEL_DEPTH_24 0x81A6
#define BS_CHANNEL_DEPTH_32 0x81A7
#define BS_CHANNEL_DEPTH_32F 0x8CAC

/* KEY CODES */
// #define BS_KEY_UNKNOWN   -1
#define BS_KEY_SPACE   32
#define BS_KEY_APOSTROPHE   39 /* ' */
#define BS_KEY_COMMA   44 /* , */
#define BS_KEY_MINUS   45 /* - */
#define BS_KEY_PERIOD   46 /* . */
#define BS_KEY_SLASH   47 /* / */
#define BS_KEY_0   48
#define BS_KEY_1   49
#define BS_KEY_2   50
#define BS_KEY_3   51
#define BS_KEY_4   52
#define BS_KEY_5   53
#define BS_KEY_6   54
#define BS_KEY_7   55
#define BS_KEY_8   56
#define BS_KEY_9   57
#define BS_KEY_SEMICOLON   59 /* ; */
#define BS_KEY_EQUAL   61 /* = */
#define BS_KEY_A   65
#define BS_KEY_B   66
#define BS_KEY_C   67
#define BS_KEY_D   68
#define BS_KEY_E   69
#define BS_KEY_F   70
#define BS_KEY_G   71
#define BS_KEY_H   72
#define BS_KEY_I   73
#define BS_KEY_J   74
#define BS_KEY_K   75
#define BS_KEY_L   76
#define BS_KEY_M   77
#define BS_KEY_N   78
#define BS_KEY_O   79
#define BS_KEY_P   80
#define BS_KEY_Q   81
#define BS_KEY_R   82
#define BS_KEY_S   83
#define BS_KEY_T   84
#define BS_KEY_U   85
#define BS_KEY_V   86
#define BS_KEY_W   87
#define BS_KEY_X   88
#define BS_KEY_Y   89
#define BS_KEY_Z   90
#define BS_KEY_LEFT_BRACKET   91 /* [ */
#define BS_KEY_BACKSLASH   92 /* \ */
#define BS_KEY_RIGHT_BRACKET   93 /* ] */
#define BS_KEY_GRAVE_ACCENT   96 /* ` */
#define BS_KEY_WORLD_1   161 /* non-US #1 */
#define BS_KEY_WORLD_2   162 /* non-US #2 */
#define BS_KEY_ESCAPE   256
#define BS_KEY_ENTER   257
#define BS_KEY_TAB   258
#define BS_KEY_BACKSPACE   259
#define BS_KEY_INSERT   260
#define BS_KEY_DELETE   261
#define BS_KEY_RIGHT   262
#define BS_KEY_LEFT   263
#define BS_KEY_DOWN   264
#define BS_KEY_UP   265
#define BS_KEY_PAGE_UP   266
#define BS_KEY_PAGE_DOWN   267
#define BS_KEY_HOME   268
#define BS_KEY_END   269
#define BS_KEY_CAPS_LOCK   280
#define BS_KEY_SCROLL_LOCK   281
#define BS_KEY_NUM_LOCK   282
#define BS_KEY_PRINT_SCREEN   283
#define BS_KEY_PAUSE   284
#define BS_KEY_F1   290
#define BS_KEY_F2   291
#define BS_KEY_F3   292
#define BS_KEY_F4   293
#define BS_KEY_F5   294
#define BS_KEY_F6   295
#define BS_KEY_F7   296
#define BS_KEY_F8   297
#define BS_KEY_F9   298
#define BS_KEY_F10   299
#define BS_KEY_F11   300
#define BS_KEY_F12   301
#define BS_KEY_F13   302
#define BS_KEY_F14   303
#define BS_KEY_F15   304
#define BS_KEY_F16   305
#define BS_KEY_F17   306
#define BS_KEY_F18   307
#define BS_KEY_F19   308
#define BS_KEY_F20   309
#define BS_KEY_F21   310
#define BS_KEY_F22   311
#define BS_KEY_F23   312
#define BS_KEY_F24   313
#define BS_KEY_F25   314
#define BS_KEY_KP_0   320
#define BS_KEY_KP_1   321
#define BS_KEY_KP_2   322
#define BS_KEY_KP_3   323
#define BS_KEY_KP_4   324
#define BS_KEY_KP_5   325
#define BS_KEY_KP_6   326
#define BS_KEY_KP_7   327
#define BS_KEY_KP_8   328
#define BS_KEY_KP_9   329
#define BS_KEY_KP_DECIMAL   330
#define BS_KEY_KP_DIVIDE   331
#define BS_KEY_KP_MULTIPLY   332
#define BS_KEY_KP_SUBTRACT   333
#define BS_KEY_KP_ADD   334
#define BS_KEY_KP_ENTER   335
#define BS_KEY_KP_EQUAL   336
#define BS_KEY_LEFT_SHIFT   340
#define BS_KEY_LEFT_CONTROL   341
#define BS_KEY_LEFT_ALT   342
#define BS_KEY_LEFT_SUPER   343
#define BS_KEY_RIGHT_SHIFT   344
#define BS_KEY_RIGHT_CONTROL   345
#define BS_KEY_RIGHT_ALT   346
#define BS_KEY_RIGHT_SUPER   347
#define BS_KEY_MENU   348
#define BS_KEY_LAST   BS_KEY_MENU

/* MEMORY BARRIERS */
#define BS_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 0x00000001
#define BS_ELEMENT_ARRAY_BARRIER_BIT 0x00000002
#define BS_UNIFORM_BARRIER_BIT 0x00000004
#define BS_TEXTURE_FETCH_BARRIER_BIT 0x00000008
#define BS_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#define BS_COMMAND_BARRIER_BIT 0x00000040
#define BS_PIXEL_BUFFER_BARRIER_BIT 0x00000080
#define BS_TEXTURE_UPDATE_BARRIER_BIT 0x00000100
#define BS_BUFFER_UPDATE_BARRIER_BIT 0x00000200
#define BS_FRAMEBUFFER_BARRIER_BIT 0x00000400
#define BS_TRANSFORM_FEEDBACK_BARRIER_BIT 0x00000800
#define BS_ATOMIC_COUNTER_BARRIER_BIT 0x00001000
#define BS_ALL_BARRIER_BITS 0xFFFFFFFF
#define BS_SHADER_STORAGE_BARRIER_BIT 0x00002000

/* BIND OPERATION */
#define BS_UNIFORM_BUFFER 0x8A11
#define BS_ATOMIC_COUNTER_BUFFER 0x92C0
#define BS_SHADER_STORAGE_BUFFER 0x90D2
#define BS_TRANSFORM_FEEDBACK_BUFFER 0x8C8E

#endif /* BS_CORE_H */