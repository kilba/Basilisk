#ifndef BS_CORE_H
#define BS_CORE_H

#include <bs_types.h>

/* --- RENDERING --- */
void bs_setVar(unsigned int v);
void bs_setV1_(float v);
void bs_setV4_(bs_vec4 v);

void bs_pushIndex(int idx);
void bs_pushIndices(int *idxs, int num_elems);
void bs_pushIndexVa(int num_elems, ...);

void bs_pushAttrib(uint8_t **data_ptr, void *data, uint8_t size);
void bs_pushVertex(
    bs_vec3  pos,
    bs_vec2  tex,
    bs_vec3  nor,
    bs_RGBA  col,
    bs_ivec4 bid,
    bs_vec4  wei
);

int bs_pushQuadFlipped(bs_vec3 p0, bs_vec3 p1, bs_vec3 p2, bs_vec3 p3, bs_RGBA col);
int bs_pushQuad(bs_vec3 p0, bs_vec3 p1, bs_vec3 p2, bs_vec3 p3, bs_RGBA col);
int bs_pushRectCoord(bs_vec3 pos, bs_vec2 dim, bs_vec2 tex_dim0, bs_vec2 tex_dim1, bs_RGBA col);
int bs_pushRectRotated(bs_vec3 pos, bs_vec2 dim, float angle, bs_RGBA col);
int bs_pushRectFlipped(bs_vec3 pos, bs_vec2 dim, bs_RGBA col);
int bs_pushRect(bs_vec3 pos, bs_vec2 dim, bs_RGBA col);
int bs_pushTriangle(bs_vec3 pos1, bs_vec3 pos2, bs_vec3 pos3, bs_RGBA color);
int bs_pushLine(bs_vec3 start, bs_vec3 end, bs_RGBA color);
int bs_pushPoint(bs_vec3 pos, bs_RGBA color);
int bs_pushAABB(bs_aabb aabb, bs_RGBA color);
int bs_pushPrim(bs_Prim *prim); 
int bs_pushMesh(bs_Mesh *mesh);
int bs_pushModel(bs_Model *model);

/* --- FRAMEBUFFERS --- */
void bs_framebuf(bs_Framebuf *framebuf, bs_ivec2 dim);
void bs_setBuf(int type, int idx, bs_Texture buf);
void bs_attachBuf(void (*tex_func)(bs_Texture *texture, bs_ivec2 dim));
void bs_attachBufExisting(bs_Texture buf, int type);
void bs_attachRenderbuf();
void bs_setDrawBufs(int n, ...);
void bs_selectFramebuf(bs_Framebuf *framebuf);
void bs_pushFramebuf();

unsigned char *bs_framebufData(int x, int y, int w, int h);
unsigned char *bs_screenshot();
void bs_screenshotFile(const char *file_name);

void bs_polygonLine();
void bs_polygonFill();
void bs_additiveBlending();
void bs_defaultBlending();

/* --- BATCHING --- */
void bs_batch(bs_Batch *batch, bs_Shader *shader);
void bs_batchRawData(void *vertex_data, void *index_data, int vertex_size, int index_size);
void bs_attrib(const int type, unsigned int amount, size_t size_per_type, bool normalized);
void bs_attribI(const int type, unsigned int amount, size_t size_per_type);
void bs_attribDivisor(int attrib_id, int value);
void bs_attribInstance(int attrib_id);

void bs_bufferRange(int target, int bind_point, int buffer, int offset, int size);
void bs_selectBatch(bs_Batch *batch);
void bs_pushBatch();
void bs_renderBatch(int start_index, int draw_count);
void bs_renderBatchVertices(int start_index, int draw_count);

void bs_minimizeBatch();
void bs_freeBatchData();
void bs_clearBatch();
void bs_batchBufferSize(int index_count, int vertex_count);

void bs_batchShader(bs_Batch *batch, bs_Shader *shader);
int bs_batchSize();
void bs_batchResizeCheck(int index_count, int vertex_count);

void bs_disableColorsI(int num, ...);
void bs_enableColorsI(int num, ...);
void bs_disableColorI(int i);
void bs_enableColorI(int i);
void bs_disableColor();
void bs_enableColor();

void bs_stencilDefault();
void bs_stencilEqZero();
void bs_stencilEqNonZero();

    // This is deprecated!
int bs_checkError();

/* --- INITIALIZATION --- */
void bs_init(int width, int height, const char *title);
void bs_startRender(void (*render)());
void bs_setGlobalVars();

/* --- MATRICES / CAMERAS --- */
bs_Camera *bs_defCamera();
void bs_setMatrices(bs_Shader *shader);
void bs_persp(bs_Camera *cam, float aspect, float fovy, float nearZ, float farZ);
void bs_ortho(bs_Camera *cam, int left, int right, int bottom, int top, float nearZ, float farZ);
void bs_lookat(bs_Camera *cam, bs_vec3 eye, bs_vec3 center, bs_vec3 up);
void bs_look(bs_Camera *cam, bs_vec3 eye, bs_vec3 dir, bs_vec3 up);

/* --- OPENGL RENDERING LOGIC LAYER --- */
void bs_lineWidth(float w);
void bs_enable(int val);
void bs_disable(int val);
void bs_stencilFunc(int val0, int val1, int val2);
void bs_depthFunc(int val);
void bs_stencilMask(int val);
void bs_depthMask(int val);
void bs_disableDepth();
void bs_enableDepth();
void bs_colorMask(int val0, int val1, int val2, int val3);
void bs_colorMaski(int i, int val0, int val1, int val2, int val3);
void bs_stencilOp(int val0, int val1, int val2);
void bs_stencilOpSeparate(int val0, int val1, int val2, int val3);
void bs_cullFace(int val);
void bs_clear(int bit_field);
void bs_clearStencil(int val);
void bs_clearColor(float r, float g, float b, float a);
void bs_frontFace(int face);
void bs_blendEquation(int val);
void bs_blendFunc(int val0, int val1);
void bs_viewport(int x, int y, int w, int h);

/* --- CONSTANTS --- */
#define BS_BATCH_INCR_BY 1024
/* OPENGL FILTERING SETTINGS */
#define BS_NEAREST 0x2600
#define BS_NEAREST_MIPMAP_LINEAR 0x2702
#define BS_NEAREST_MIPMAP_NEAREST 0x2700

#define BS_LINEAR 0x2601
#define BS_LINEAR_MIPMAP_LINEAR 0x2703
#define BS_LINEAR_MIPMAP_NEAREST 0x2701

#define BS_TEXTURE_WRAP_S 0x2802
#define BS_TEXTURE_WRAP_T 0x2803
#define BS_TEXTURE_WRAP_R 0x8072
#define BS_TEXTURE_WRAP_ST 0x1000
#define BS_TEXTURE_WRAP_STR 0x2000

#define BS_CLAMP_TO_EDGE 0x812F

/* TEXTURE TYPES */
#define BS_TEX1D 0x0DE0
#define BS_TEX2D 0x0DE1
#define BS_CUBEMAP 0x8513

#define BS_COLOR 0x8CE0
#define BS_DEPTH 0x8D00
#define BS_STENCIL 0x8D20
#define BS_DEPTH_STENCIL 0x821A

/* CLEAR BUFFER BITS */
#define BS_DEPTH_BUFFER_BIT 0x00000100
#define BS_STENCIL_BUFFER_BIT 0x00000400
#define BS_COLOR_BUFFER_BIT 0x00004000

/* FACES */
#define BS_FACE_FRONT_RIGHT 0x0401
#define BS_FACE_BACK_LEFT 0x0402
#define BS_FACE_BACK_RIGHT 0x0403
#define BS_FACE_FRONT 0x0404
#define BS_FACE_BACK 0x0405
#define BS_FACE_LEFT 0x0406
#define BS_FACE_RIGHT 0x0407
#define BS_FACE_FRONT_AND_BACK 0x0408

#define BS_CW  0x0900
#define BS_CCW 0x0901

/* BATCH ATTRIBUTE TYPES  */
#define BS_VAL_POS 1
#define BS_VAL_TEX 2
#define BS_VAL_COL 4
#define BS_VAL_NOR 8
#define BS_VAL_BID 16
#define BS_VAL_WEI 32
#define BS_VAL_VAR 64
#define BS_VAL_V4_ 128
#define BS_VAL_V1_ 256

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

/* RENDERING LOGIC */
#define BS_STENCIL_TEST 0x0B90
#define BS_DEPTH_TEST 0x0B71
#define BS_BLEND 0x0BE2

#define BS_CULL_FACE 0x0B44
#define BS_DEPTH_CLAMP 0x864F

#define BS_NEVER 0x0200
#define BS_LESS 0x0201
#define BS_EQUAL 0x0202
#define BS_LEQUAL 0x0203
#define BS_GREATER 0x0204
#define BS_NOTEQUAL 0x0205
#define BS_GEQUAL 0x0206
#define BS_ALWAYS 0x0207

#define BS_KEEP 0x1E00
#define BS_REPLACE 0x1E01
#define BS_INCR 0x1E02
#define BS_DECR 0x1E03
#define BS_INCR_WRAP 0x8507
#define BS_DECR_WRAP 0x8508

#define BS_FUNC_ADD 0x8006
#define BS_FUNC_REVERSE_SUBTRACT 0x800B
#define BS_FUNC_SUBTRACT 0x800A
#define BS_MIN 0x8007
#define BS_MAX 0x8008

#define BS_ZERO 0
#define BS_ONE 1
#define BS_SRC_COLOR 0x0300
#define BS_ONE_MINUS_SRC_COLOR 0x0301
#define BS_SRC_ALPHA 0x0302
#define BS_ONE_MINUS_SRC_ALPHA 0x0303
#define BS_DST_ALPHA 0x0304
#define BS_ONE_MINUS_DST_ALPHA 0x0305
#define BS_DST_COLOR 0x0306
#define BS_ONE_MINUS_DST_COLOR 0x0307
#define BS_CONSTANT_ALPHA 0x8003
#define BS_ONE_MINUS_CONSTANT_ALPHA 0x8004

/* DATATYPES */
#define BS_SBYTE 0x1400
#define BS_UBYTE 0x1401
#define BS_SHORT 0x1402
#define BS_USHORT 0x1403
#define BS_INT 0x1404
#define BS_UINT 0x1405
#define BS_FLOAT 0x1406

#define BS_UINT24_8 0x84FA

/* BASE INTERNAL FORMATS */
#define BS_CHANNEL_RED 0x1903
#define BS_CHANNEL_GREEN 0x1904
#define BS_CHANNEL_BLUE 0x1905
#define BS_CHANNEL_ALPHA 0x1906
#define BS_CHANNEL_RGB 0x1907
#define BS_CHANNEL_RGBA 0x1908
#define BS_CHANNEL_DEPTH24_STENCIL8 0x88F0

/* SIZED INTERNAL FORMATS */
#define BS_CHANNEL_RGBA32F 0x8814
#define BS_CHANNEL_RGB32F 0x8815
#define BS_CHANNEL_RGBA16F 0x881A
#define BS_CHANNEL_RGB16F 0x881B
#define BS_CHANNEL_DEPTH_STENCIL 0x84F9

/* DEPTH INTERNAL FORMATS */
#define BS_CHANNEL_DEPTH 0x1902
#define BS_CHANNEL_DEPTH_16 0x81A5
#define BS_CHANNEL_DEPTH_24 0x81A6
#define BS_CHANNEL_DEPTH_32 0x81A7
#define BS_CHANNEL_DEPTH_32F 0x8CAC

/* KEY CODES */
#define BS_KEY_LBUTTON 0x01
#define BS_KEY_RBUTTON 0x02
#define BS_KEY_CANCEL 0x03
#define BS_KEY_MBUTTON 0x04
#define BS_KEY_XBUTTON1 0x05
#define BS_KEY_XBUTTON2 0x06
#define BS_KEY_BACK 0x08
#define BS_KEY_TAB 0x09
#define BS_KEY_CLEAR 0x0C

#define BS_KEY_RETURN 0x0D
#define BS_KEY_SHIFT 0x10
#define BS_KEY_CONTROL 0x11
#define BS_KEY_MENU 0x12
#define BS_KEY_PAUSE 0x13
#define BS_KEY_CAPITAL 0x14
#define BS_KEY_ENTER BS_KEY_RETURN
#define BS_KEY_CTRL BS_KEY_CONTROL

#define BS_KEY_KANA 0x15
#define BS_KEY_JUNJA 0x17
#define BS_KEY_FINAL 0x18
#define BS_KEY_HANJA 0x19
#define BS_KEY_KANJI BS_KEY_HANJA

#define BS_KEY_ESCAPE 0x1B
#define BS_KEY_CONVERT 0x1C
#define BS_KEY_NONCONVERT 0x1D
#define BS_KEY_ACCEPT 0x1E
#define BS_KEY_MODECHANGE 0x1F
#define BS_KEY_SPACE 0x20
#define BS_KEY_PRIOR 0x21
#define BS_KEY_NEXT 0x22
#define BS_KEY_END 0x23
#define BS_KEY_HOME 0x24
#define BS_KEY_LEFT 0x25
#define BS_KEY_UP 0x26
#define BS_KEY_RIGHT 0x27
#define BS_KEY_DOWN 0x28
#define BS_KEY_SELECT 0x29
#define BS_KEY_PRINT 0x2A
#define BS_KEY_EXECUTE 0x2B
#define BS_KEY_SNAPSHOT 0x2C
#define BS_KEY_INSERT 0x2D
#define BS_KEY_DELETE 0x2E
#define BS_KEY_HELP 0x2F
#define BS_KEY_0 0x30
#define BS_KEY_1 0x31
#define BS_KEY_2 0x32
#define BS_KEY_3 0x33
#define BS_KEY_4 0x34
#define BS_KEY_5 0x35
#define BS_KEY_6 0x36
#define BS_KEY_7 0x37
#define BS_KEY_8 0x38
#define BS_KEY_9 0x39
#define BS_KEY_A 0x41
#define BS_KEY_B 0x42
#define BS_KEY_C 0x43
#define BS_KEY_D 0x44
#define BS_KEY_E 0x45
#define BS_KEY_F 0x46
#define BS_KEY_G 0x47
#define BS_KEY_H 0x48
#define BS_KEY_I 0x49
#define BS_KEY_J 0x4A
#define BS_KEY_K 0x4B
#define BS_KEY_L 0x4C
#define BS_KEY_M 0x4D
#define BS_KEY_N 0x4E
#define BS_KEY_O 0x4F
#define BS_KEY_P 0x50
#define BS_KEY_Q 0x51
#define BS_KEY_R 0x52
#define BS_KEY_S 0x53
#define BS_KEY_T 0x54
#define BS_KEY_U 0x55
#define BS_KEY_V 0x56
#define BS_KEY_W 0x57
#define BS_KEY_X 0x58
#define BS_KEY_Y 0x59
#define BS_KEY_Z 0x5A
#define BS_KEY_LWIN 0x5B
#define BS_KEY_RWIN 0x5C
#define BS_KEY_APPS 0x5D
#define BS_KEY_SLEEP 0x5F
#define BS_KEY_NUMPAD0 0x60
#define BS_KEY_NUMPAD1 0x61
#define BS_KEY_NUMPAD2 0x62
#define BS_KEY_NUMPAD3 0x63
#define BS_KEY_NUMPAD4 0x64
#define BS_KEY_NUMPAD5 0x65
#define BS_KEY_NUMPAD6 0x66
#define BS_KEY_NUMPAD7 0x67
#define BS_KEY_NUMPAD8 0x68
#define BS_KEY_NUMPAD9 0x69
#define BS_KEY_MULTIPLY 0x6A
#define BS_KEY_ADD 0x6B
#define BS_KEY_SEPARATOR 0x6C
#define BS_KEY_SUBTRACT 0x6D
#define BS_KEY_DECIMAL 0x6E
#define BS_KEY_DIVIDE 0x6F
#define BS_KEY_F1 0x70
#define BS_KEY_F2 0x71
#define BS_KEY_F3 0x72
#define BS_KEY_F4 0x73
#define BS_KEY_F5 0x74
#define BS_KEY_F6 0x75
#define BS_KEY_F7 0x76
#define BS_KEY_F8 0x77
#define BS_KEY_F9 0x78
#define BS_KEY_F10 0x79
#define BS_KEY_F11 0x7A
#define BS_KEY_F12 0x7B
#define BS_KEY_F13 0x7C
#define BS_KEY_F14 0x7D
#define BS_KEY_F15 0x7E
#define BS_KEY_F16 0x7F
#define BS_KEY_F17 0x80
#define BS_KEY_F18 0x81
#define BS_KEY_F19 0x82
#define BS_KEY_F20 0x83
#define BS_KEY_F21 0x84
#define BS_KEY_F22 0x85
#define BS_KEY_F23 0x86
#define BS_KEY_F24 0x87
#define BS_KEY_NUMLOCK 0x90
#define BS_KEY_SCROLL 0x91
#define BS_KEY_LSHIFT 0xA0
#define BS_KEY_RSHIFT 0xA1
#define BS_KEY_LCONTROL 0xA2
#define BS_KEY_RCONTROL 0xA3
#define BS_KEY_LMENU 0xA4
#define BS_KEY_RMENU 0xA5
#define BS_KEY_BROWSER_BACK 0xA6
#define BS_KEY_BROWSER_FORWARD 0xA7
#define BS_KEY_BROWSER_REFRESH 0xA8
#define BS_KEY_BROWSER_STOP 0xA9
#define BS_KEY_BROWSER_SEARCH 0xAA
#define BS_KEY_BROWSER_FAVORITES 0xAB
#define BS_KEY_BROWSER_HOME 0xAC
#define BS_KEY_VOLUME_MUTE 0xAD
#define BS_KEY_VOLUME_DOWN 0xAE
#define BS_KEY_VOLUME_UP 0xAF
#define BS_KEY_MEDIA_NEXT_TRACK 0xB0
#define BS_KEY_MEDIA_PREV_TRACK 0xB1
#define BS_KEY_MEDIA_STOP 0xB2
#define BS_KEY_MEDIA_PLAY_PAUSE 0xB3
#define BS_KEY_LAUNCH_MAIL 0xB4
#define BS_KEY_LAUNCH_MEDIA_SELECT 0xB5
#define BS_KEY_LAUNCH_APP1 0xB6
#define BS_KEY_LAUNCH_APP2 0xB7
#define BS_KEY_OEM_1 0xBA
#define BS_KEY_OEM_PLUS 0xBB
#define BS_KEY_OEM_COMMA 0xBC
#define BS_KEY_OEM_MINUS 0xBD
#define BS_KEY_OEM_PERIOD 0xBE
#define BS_KEY_OEM_2 0xBF
#define BS_KEY_OEM_3 0xC0
#define BS_KEY_OEM_4 0xDB
#define BS_KEY_OEM_5 0xDC
#define BS_KEY_OEM_6 0xDD
#define BS_KEY_OEM_7 0xDE
#define BS_KEY_OEM_8 0xDF
#define BS_KEY_OEM_102 0xE2
#define BS_KEY_PROCESSKEY 0xE5
#define BS_KEY_PACKET 0xE7
#define BS_KEY_ATTN 0xF6
#define BS_KEY_CRSEL 0xF7
#define BS_KEY_EXSEL 0xF8
#define BS_KEY_EREOF 0xF9
#define BS_KEY_PLAY 0xFA
#define BS_KEY_ZOOM 0xFB
#define BS_KEY_NONAME 0xFC
#define BS_KEY_PA1 0xFD
#define BS_KEY_OEM_CLEAR 0xFE  

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
