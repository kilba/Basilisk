// GL
#include "bs_types.h"
#include "lodepng.h"
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

bs_Tex2D *curr_texture;

/* TEXTURE INITIALIZATION */
void bs_texture(bs_Tex2D *texture, bs_ivec2 dim, int type) {
    glGenTextures(1, &texture->id);
    glBindTexture(type, texture->id);

    curr_texture = texture;
    curr_texture->w = dim.x;
    curr_texture->h = dim.y;
    curr_texture->data = NULL;
    curr_texture->type = type;
}

void bs_textureDataRaw(unsigned char *data) {
    curr_texture->data = data;
}

void bs_textureDataFile(char *path, bool update_dimensions) {
    unsigned int w, h;
    int success = lodepng_decode32_file(&curr_texture->data, &w, &h, path);

    if(success != 0)
        return;

    if(update_dimensions) {
        curr_texture->w = w;
        curr_texture->h = h;
    }
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

void bs_selectTexture(bs_Tex2D *texture, int tex_unit) {
    curr_texture = texture;
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(curr_texture->type, texture->id);
}

/* Functions for easier texture initialization */
void bs_depth(bs_Tex2D *texture, bs_ivec2 dim) {
    bs_texture(texture, dim, BS_TEX2D);
    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_pushTexture(BS_CHANNEL_DEPTH, BS_CHANNEL_DEPTH, BS_FLOAT);
}

void bs_depthLin(bs_Tex2D *texture, bs_ivec2 dim) {
    bs_texture(texture, dim, BS_TEX2D);
    bs_textureMinMag(BS_LINEAR, BS_LINEAR);
    bs_pushTexture(BS_CHANNEL_DEPTH, BS_CHANNEL_DEPTH, BS_FLOAT);
}

void bs_textureRGBA(bs_Tex2D *texture, bs_ivec2 dim) {
    bs_texture(texture, dim, BS_TEX2D);
    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
}

void bs_textureLinRGBA(bs_Tex2D *texture, bs_ivec2 dim) {
    bs_texture(texture, dim, BS_TEX2D);
    bs_textureMinMag(BS_LINEAR, BS_LINEAR);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
}

void bs_texturePNG(bs_Tex2D *texture, char *path) {
    bs_texture(texture, BS_CIVEC2(0, 0), BS_TEX2D);
    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_textureDataFile(path, true);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
}

void bs_textureLinPNG(bs_Tex2D *texture, char *path) {
    bs_texture(texture, BS_CIVEC2(0, 0), BS_TEX2D);
    bs_textureMinMag(BS_LINEAR, BS_LINEAR);
    bs_textureDataFile(path, true);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
}

void bs_depthCube(bs_Tex2D *texture, int dim) {
    bs_texture(texture, BS_CIVEC2(dim, dim), BS_CUBEMAP);

    for(int i = 0; i < 6; ++i) {
	bs_pushTextureTarget(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, BS_CHANNEL_DEPTH, BS_CHANNEL_DEPTH, BS_FLOAT);
    }

    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_textureWrap(BS_TEXTURE_WRAP_STR, BS_CLAMP_TO_EDGE);
}

void bs_textureCube(bs_Tex2D *texture, int dim, char *paths[6]) {
    bs_texture(texture, BS_CIVEC2(dim, dim), BS_CUBEMAP);

    for(int i = 0; i < 6; ++i) {
	bs_textureDataFile(paths[i], true);
	bs_pushTextureTarget(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
    }

    bs_textureMinMag(BS_NEAREST, BS_NEAREST);
    bs_textureWrap(BS_TEXTURE_WRAP_STR, BS_CLAMP_TO_EDGE);
}
