// GL
#include "bs_types.h"
#include <glad/glad.h>
#include <cglm/cglm.h>

// Basilisk
#include <bs_shaders.h>
#include <bs_core.h>
#include <bs_textures.h>

#include <lodepng.h>

// STD
#include <string.h>
#include <stddef.h>
#include <windows.h>

bs_Tex2D *curr_texture;

/* TEXTURE INITIALIZATION */
void bs_texture(bs_Tex2D *texture, int w, int h) {
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    curr_texture = texture;
    curr_texture->w = w;
    curr_texture->h = h;
    curr_texture->data = NULL;
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

void bs_textureSettings(int min_filter, int mag_filter) {
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
}

void bs_pushTexture(int internal_format, int format, int type) {
    glTexImage2D(
        GL_TEXTURE_2D, 
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

void bs_textureMipmaps() {
    glGenerateMipmap(GL_TEXTURE_2D);
}

void bs_selectTexture(bs_Tex2D *texture, int tex_unit) {
    curr_texture = texture;
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(GL_TEXTURE_2D, texture->id);
}

/* Functions for easier texture initialization */
void bs_depth(bs_Tex2D *texture, int w, int h) {
    bs_texture(texture, w, h);
    bs_textureSettings(BS_NEAREST, BS_NEAREST);
    bs_pushTexture(BS_CHANNEL_DEPTH, BS_CHANNEL_DEPTH, BS_FLOAT);
}

void bs_depthLin(bs_Tex2D *texture, int w, int h) {
    bs_texture(texture, w, h);
    bs_textureSettings(BS_LINEAR, BS_LINEAR);
    bs_pushTexture(BS_CHANNEL_DEPTH, BS_CHANNEL_DEPTH, BS_FLOAT);
}

void bs_textureRGBA(bs_Tex2D *texture, int w, int h) {
    bs_texture(texture, w, h);
    bs_textureSettings(BS_NEAREST, BS_NEAREST);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
}

void bs_textureLinRGBA(bs_Tex2D *texture, int w, int h) {
    bs_texture(texture, w, h);
    bs_textureSettings(BS_LINEAR, BS_LINEAR);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
}

void bs_texturePNG(bs_Tex2D *texture, char *path) {
    bs_texture(texture, 0, 0);
    bs_textureSettings(BS_NEAREST, BS_NEAREST);
    bs_textureDataFile(path, true);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
}

void bs_textureLinPNG(bs_Tex2D *texture, char *path) {
    bs_texture(texture, 0, 0);
    bs_textureSettings(BS_LINEAR, BS_LINEAR);
    bs_textureDataFile(path, true);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
}
