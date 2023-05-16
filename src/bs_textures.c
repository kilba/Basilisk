// GL
#include <glad/glad.h>
#include <cglm/cglm.h>

// Basilisk
#include <bs_shaders.h>
#include <bs_core.h>
#include <bs_textures.h>

#include <lodepng.h>

// STD
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <windows.h>
#include <winreg.h>

bs_Texture *curr_texture;
int filter = BS_NEAREST;

void bs_saveTexture32(const char *name, unsigned char *data, int w, int h) {
    lodepng_encode32_file(name, data, w, h);
}

/* TEXTURE INITIALIZATION */
void bs_texture(bs_Texture *texture, bs_ivec2 dim, int type) {
    glGenTextures(1, &texture->id);
    glBindTexture(type, texture->id);

    curr_texture = texture;
    curr_texture->frame = BS_IV3_0;
    curr_texture->num_frames = 1;

    curr_texture->w = dim.x;
    curr_texture->h = dim.y;

    curr_texture->texw = 1.0;
    curr_texture->texh = 1.0;

    curr_texture->data = NULL;
    curr_texture->type = type;
}

void bs_textureDataRaw(unsigned char *data) {
    curr_texture->data = data;
}

bs_U32 bs_textureDataFile(const char *path, bool update_dimensions) {
    unsigned int w, h;
    bs_U32 err = lodepng_decode32_file(&curr_texture->data, &w, &h, path);

    if(err != 0)
        return err;

    if(update_dimensions) {
        curr_texture->w = w;
        curr_texture->h = h;
    }
    return 0;
}

void bs_textureMinMag(int min_filter, int mag_filter) {
    glTexParameteri(curr_texture->type, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(curr_texture->type, GL_TEXTURE_MAG_FILTER, mag_filter);
}

void bs_textureWrap(int mode, int value) {
    if(mode == BS_TEXTURE_WRAP_ST) {
	glTexParameteri(curr_texture->type, BS_TEXTURE_WRAP_S, value);
	glTexParameteri(curr_texture->type, BS_TEXTURE_WRAP_T, value);

	return;
    }

    if(mode == BS_TEXTURE_WRAP_STR) {
	glTexParameteri(curr_texture->type, BS_TEXTURE_WRAP_S, value);
	glTexParameteri(curr_texture->type, BS_TEXTURE_WRAP_T, value);
	glTexParameteri(curr_texture->type, BS_TEXTURE_WRAP_R, value);

	return;
    }

    glTexParameteri(curr_texture->type, mode, value);
}

void bs_pushTextureTarget(int target, int internal_format, int format, int type) {
    glTexImage2D(
        target, 
        0, 
        internal_format, 
        curr_texture->w, 
        curr_texture->h, 
        0, 
        format, 
        type,
        curr_texture->data
    );
}

void bs_pushTexture(int internal_format, int format, int type) {
    bs_pushTextureTarget(curr_texture->type, internal_format, format, type);
}

void bs_textureHandle() {
    curr_texture->handle = glGetTextureHandleARB(curr_texture->id);
    glMakeTextureHandleResidentARB(curr_texture->handle);
}

void bs_textureMipmaps() {
    glGenerateMipmap(curr_texture->type);
}

void bs_textureSplit(int frame_count) {
    bs_Texture *tex = curr_texture;
    tex->num_frames = frame_count;
    tex->texw = 1.0 / (float)frame_count;
}

void bs_textureSplitVert(int frame_count) {
    bs_Texture *tex = curr_texture;
    tex->num_frames = frame_count;
    tex->texh = 1.0 / (float)frame_count;
}

void bs_setTexture(bs_Texture *texture) {
    curr_texture = texture;
}

void bs_selectTextureTarget(bs_Texture *texture, int tex_unit, int target) {
    if(curr_texture == texture)
	return;

    curr_texture = texture;
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(target, texture->id);
}

void bs_selectTexture(bs_Texture *texture, int tex_unit) {
    bs_selectTextureTarget(texture, tex_unit, texture->type);
}

bs_Texture *bs_selectedTexture() {
    return curr_texture;
}

void bs_textureDataRGBA(bs_Texture *texture, unsigned char *data, bs_ivec2 dim) {
    if(data == NULL)
	return;

    bs_texture(texture, dim, BS_TEX2D);
    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_textureDataRaw(data);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
    bs_textureHandle();
}

void bs_texturePNG(bs_Texture *texture, char *path) {
    if(path == NULL)
	return;

    bs_texture(texture, BS_IVEC2(0, 0), BS_TEX2D);
    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_textureDataFile(path, true);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
    bs_textureHandle();
}

void bs_textureLinPNG(bs_Texture *texture, char *path) {
    if(path == NULL)
	return;

    bs_texture(texture, BS_IVEC2(0, 0), BS_TEX2D);
    bs_textureMinMag(BS_LINEAR, BS_LINEAR);
    bs_textureDataFile(path, true);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
}

void bs_depthCube(bs_Texture *texture, int dim) {
    bs_texture(texture, BS_IVEC2(dim, dim), BS_CUBEMAP);

    for(int i = 0; i < 6; ++i) {
	bs_pushTextureTarget(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, BS_CHANNEL_DEPTH, BS_CHANNEL_DEPTH, BS_FLOAT);
    }

    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_textureWrap(BS_TEXTURE_WRAP_STR, BS_CLAMP_TO_EDGE);
}

void bs_depthCubeLin(bs_Texture *texture, int dim) {
    bs_texture(texture, BS_IVEC2(dim, dim), BS_CUBEMAP);

    for(int i = 0; i < 6; ++i) {
	bs_pushTextureTarget(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, BS_CHANNEL_DEPTH, BS_CHANNEL_DEPTH, BS_FLOAT);
    }

    bs_textureMinMag(BS_LINEAR, BS_LINEAR);
    bs_textureWrap(BS_TEXTURE_WRAP_STR, BS_CLAMP_TO_EDGE);
}

void bs_textureCube(bs_Texture *texture, int dim, char *paths[6]) {
    bs_texture(texture, BS_IVEC2(dim, dim), BS_CUBEMAP);

    for(int i = 0; i < 6; ++i) {
	bs_textureDataFile(paths[i], true);
	bs_pushTextureTarget(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
    }

    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_textureWrap(BS_TEXTURE_WRAP_STR, BS_CLAMP_TO_EDGE);
}

void bs_textureCubeLin(bs_Texture *texture, int dim, char *paths[6]) {
    bs_texture(texture, BS_IVEC2(dim, dim), BS_CUBEMAP);

    for(int i = 0; i < 6; ++i) {
	bs_textureDataFile(paths[i], true);
	bs_pushTextureTarget(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
    }

    bs_textureMinMag(BS_LINEAR, BS_LINEAR);
    bs_textureWrap(BS_TEXTURE_WRAP_STR, BS_CLAMP_TO_EDGE);
}

void bs_textureArray(bs_Texture *tex, bs_ivec2 max_dim, int num_textures) {
    bs_texture(tex, max_dim, GL_TEXTURE_2D_ARRAY);

    glTexStorage3D(
	GL_TEXTURE_2D_ARRAY,
	1,
	GL_RGBA8,
	max_dim.x, max_dim.y,
	num_textures
    );
    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
}

bs_U32 bs_textureArrayInsertPNG(const char *path, int idx) {
    bs_U32 err = bs_textureDataFile(path, true);
    if(err != 0)
	return err;

    glTexSubImage3D(
	curr_texture->type,
	0,
	0, 0, idx,
	curr_texture->w, curr_texture->h, 1,
	BS_CHANNEL_RGBA,
	BS_UBYTE,
	curr_texture->data
    );

    curr_texture->frame.z++;
    free(curr_texture->data);
    return 0;
}

bs_U32 bs_textureArrayAppendPNG(const char *path) {
    return bs_textureArrayInsertPNG(path, curr_texture->frame.z);
}

bs_U32 bs_textureArrayInsertPNGSheet(const char *path, int frames, int idx) {
    bs_U32 err = bs_textureDataFile(path, true);
    if(err != 0)
	return err;

    int frame_width = curr_texture->w / frames;
    glPixelStorei(GL_UNPACK_ROW_LENGTH, curr_texture->w);
    for(int i = 0; i < frames; i++) {
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, i * frame_width);
	glTexSubImage3D(
	    curr_texture->type,
	    0,
	    0, 0, curr_texture->frame.z,
	    frame_width, curr_texture->h, 1,
	    BS_CHANNEL_RGBA,
	    BS_UBYTE,
	    curr_texture->data
	);
	curr_texture->frame.z++;
    }
    
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    free(curr_texture->data);

    return 0;
}

bs_U32 bs_textureArrayAppendPNGSheet(const char *path, int frames) {
    return bs_textureArrayInsertPNGSheet(path, frames, curr_texture->frame.z);
}

void bs_linearFiltering() {
    filter = BS_LINEAR;
}

void bs_nearestFiltering() {
    filter = BS_NEAREST;
}

void bs_textureColor(bs_Texture *texture, bs_ivec2 dim, int internal_format, int format, int type) {
    bs_texture(texture, dim, BS_TEX2D);
    bs_textureMinMag(filter, filter);
    bs_pushTexture(internal_format, format, type);
}

void bs_depthStencil(bs_Texture *texture, bs_ivec2 dim) {
    bs_textureColor(texture, dim, BS_CHANNEL_DEPTH24_STENCIL8, BS_CHANNEL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
    texture->attachment = BS_DEPTH_STENCIL;
}

void bs_depth(bs_Texture *texture, bs_ivec2 dim) {
    bs_textureColor(texture, dim, BS_CHANNEL_DEPTH, BS_CHANNEL_DEPTH, BS_FLOAT);
    texture->attachment = BS_DEPTH;
}

/* --- FLOAT TEXTURES --- */
void bs_textureRG(bs_Texture *texture, bs_ivec2 dim) {
    bs_textureColor(texture, dim, GL_RG8, GL_RG, BS_UBYTE);
    texture->attachment = BS_COLOR;
}

void bs_textureRGB(bs_Texture *texture, bs_ivec2 dim) {
    bs_textureColor(texture, dim, GL_RGB8, BS_CHANNEL_RGB, BS_UBYTE);
    texture->attachment = BS_COLOR;
}

void bs_textureRGB16f(bs_Texture *texture, bs_ivec2 dim) {
    bs_textureColor(texture, dim, BS_CHANNEL_RGB16F, BS_CHANNEL_RGB, BS_FLOAT);
    texture->attachment = BS_COLOR;
}

void bs_textureRGB32f(bs_Texture *texture, bs_ivec2 dim) {
    bs_textureColor(texture, dim, BS_CHANNEL_RGB32F, BS_CHANNEL_RGB, BS_FLOAT);
    texture->attachment = BS_COLOR;
}

void bs_textureRGBA(bs_Texture *texture, bs_ivec2 dim) {
    bs_textureColor(texture, dim, GL_RGBA8, BS_CHANNEL_RGBA, BS_UBYTE);
    texture->attachment = BS_COLOR;
}

void bs_textureRGBA16f(bs_Texture *texture, bs_ivec2 dim) {
    bs_textureColor(texture, dim, GL_RGBA16F, GL_RGBA, BS_FLOAT);
    texture->attachment = BS_COLOR;
}

void bs_textureRGBA32f(bs_Texture *texture, bs_ivec2 dim) {
    bs_textureColor(texture, dim, GL_RGBA32F, GL_RGBA, BS_FLOAT);
    texture->attachment = BS_COLOR;
}

void bs_texture_11_11_10(bs_Texture *texture, bs_ivec2 dim) {
    bs_textureColor(texture, dim, GL_R11F_G11F_B10F, GL_RGB, BS_FLOAT);
    texture->attachment = BS_COLOR;
}

void bs_textureR16U(bs_Texture *texture, bs_ivec2 dim) {
    bs_textureColor(texture, dim, GL_RGB16UI, GL_RED_INTEGER, BS_UINT);
    texture->attachment = BS_COLOR;
}
