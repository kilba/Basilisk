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

bs_U32 bs_textureDataFile(char *path, bool update_dimensions) {
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

/* Functions for easier texture initialization */
void bs_depth(bs_Texture *texture, bs_ivec2 dim) {
    bs_texture(texture, dim, BS_TEX2D);
    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_pushTexture(BS_CHANNEL_DEPTH, BS_CHANNEL_DEPTH, BS_FLOAT);
}

void bs_depthLin(bs_Texture *texture, bs_ivec2 dim) {
    bs_texture(texture, dim, BS_TEX2D);
    bs_textureMinMag(BS_LINEAR, BS_LINEAR);
    bs_pushTexture(BS_CHANNEL_DEPTH, BS_CHANNEL_DEPTH, BS_FLOAT);
}

void bs_textureRGBA(bs_Texture *texture, bs_ivec2 dim) {
    bs_texture(texture, dim, BS_TEX2D);
    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
}

void bs_textureLinRGBA(bs_Texture *texture, bs_ivec2 dim) {
    bs_texture(texture, dim, BS_TEX2D);
    bs_textureMinMag(BS_LINEAR, BS_LINEAR);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
}

void bs_texturePNG(bs_Texture *texture, char *path) {
    if(path == NULL)
	return;

    bs_texture(texture, BS_IVEC2(0, 0), BS_TEX2D);
    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_textureDataFile(path, true);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
}

void bs_textureLinPNG(bs_Texture *texture, char *path) {
    if(path == NULL)
	return;

    bs_texture(texture, BS_IVEC2(0, 0), BS_TEX2D);
    bs_textureMinMag(BS_LINEAR, BS_LINEAR);
    bs_textureDataFile(path, true);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
}

void bs_depthStencil(bs_Texture *texture, bs_ivec2 dim) {
    bs_texture(texture, dim, BS_TEX2D);
    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_pushTexture(BS_CHANNEL_DEPTH24_STENCIL8, BS_CHANNEL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
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

bs_U32 bs_textureArrayAppendPNG(char *path) {
    bs_U32 err = bs_textureDataFile(path, true);
    if(err != 0)
	return err;

    glTexSubImage3D(
	curr_texture->type,
	0,
	0, 0, curr_texture->frame.z,
	curr_texture->w, curr_texture->h, 1,
	BS_CHANNEL_RGBA,
	BS_UBYTE,
	curr_texture->data
    );

    curr_texture->frame.z++;
    return 0;
}

void bs_textureRGBA16f(bs_Texture *tex, bs_ivec2 dim) {
    bs_texture(tex, dim, BS_TEX2D);
    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_pushTexture(GL_RGBA16F, GL_RGBA, BS_FLOAT);
}

void bs_textureLinRGBA16f(bs_Texture *tex, bs_ivec2 dim) {
    bs_texture(tex, dim, BS_TEX2D);
    bs_textureMinMag(BS_LINEAR, BS_LINEAR);
    bs_pushTexture(GL_RGBA16F, GL_RGBA, BS_FLOAT);
}

void bs_textureRGBA32f(bs_Texture *tex, bs_ivec2 dim) {
    bs_texture(tex, dim, BS_TEX2D);
    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_pushTexture(GL_RGBA32F, GL_RGBA, BS_FLOAT);
}

void bs_textureLinRGBA32f(bs_Texture *tex, bs_ivec2 dim) {
    bs_texture(tex, dim, BS_TEX2D);
    bs_textureMinMag(BS_LINEAR, BS_LINEAR);
    bs_pushTexture(GL_RGBA32F, GL_RGBA, BS_FLOAT);
}

void bs_texture_11_11_10(bs_Texture *tex, bs_ivec2 dim) {
    bs_texture(tex, dim, BS_TEX2D);
    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_pushTexture(GL_R11F_G11F_B10F, GL_RGB, BS_FLOAT);
}

void bs_textureLin_11_11_10(bs_Texture *tex, bs_ivec2 dim) {
    bs_texture(tex, dim, BS_TEX2D);
    bs_textureMinMag(BS_LINEAR, BS_LINEAR);
    bs_pushTexture(GL_R11F_G11F_B10F, GL_RGB, BS_FLOAT);
}
