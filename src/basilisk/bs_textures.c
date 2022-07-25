// GL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

// Basilisk
#include <bs_shaders.h>
#include <bs_core.h>
#include <bs_textures.h>

#include <lodepng.h>
#include <cappend.h>
#include <rectpacker.h>

// STD
#include <string.h>
#include <stddef.h>
#include <windows.h>

int atlas_count = 0;
bs_Atlas *atlases;
bs_TextureSlice *curr_texture_slice;
bs_Texture *curr_texture;

void bs_splitTexture(unsigned char *data, int w, int h, int frames, int *curr_tex_count, bs_TextureSlice **textures) {
    int slice_width = w / frames; // TODO: Check if not an int

    // For each frame
    for(int i = 0; i < frames; i++) {
        bs_TextureSlice *tex = (*textures) + *curr_tex_count + i;
        tex->w = w;
        tex->h = h;
        tex->x = 0;
        tex->y = 0;

        int row_size_slice = 4 * slice_width;
        int row_size_full = 4 * w;
        tex->data = malloc(4 * slice_width * h);

        // For each horizontal slice
        for(int j = 0; j < h; j++) {
            int write_offset = j * row_size_slice;
            int read_offset = j * row_size_full;
            read_offset += i * row_size_slice;
            memcpy(tex->data + write_offset, data + read_offset, row_size_slice);
        }
        cappend_ImgInfo img_info;

        cappend_getMinimumTextureSize(tex->data, slice_width, tex->h, &img_info);
        cappend_removePixelsByExtent(&tex->data, slice_width, tex->h, &img_info);

        tex->w = img_info.w;
        tex->h = img_info.h;
    }
}

void bs_setOffsets(int width, int height, bs_Atlas *atlas) {
    rectpacker_Rect *rects = malloc(sizeof(rectpacker_Rect) * atlas->tex_count);
    bs_TextureSlice *tex = atlas->textures;

    for(int i = 0; i < atlas->tex_count; i++) {
        rects[i].w = tex[i].w;
        rects[i].h = tex[i].h;
    }

    rectpacker_packRect(rects, atlas->tex_count, width, height);

    for(int i = 0; i < atlas->tex_count; i++) {
        tex[i].x = rects[i].x;
        tex[i].y = rects[i].y;
        tex[i].tex_x = rects[i].tex_x;
        tex[i].tex_y = rects[i].tex_y;
        tex[i].tex_wx = rects[i].tex_x + rects[i].w / (float)width;
        tex[i].tex_hy = rects[i].tex_y + rects[i].h / (float)height;
    }
}

void bs_appendToAtlas(unsigned char *atlas_data, int width, int height, bs_Atlas *atlas) {
    for(int i = 0; i < atlas->tex_count; i++) {
        bs_TextureSlice *tex = &atlas->textures[i];
        cappend_append(atlas_data, width, height, tex->data, tex->w, tex->h, tex->x, tex->y);
        free(tex->data);
        tex->data = NULL;
    }
}

void bs_createWhiteSquare(int dim, bs_Atlas *atlas) {
    for(int i = 0; i < dim; i++) {
        int pos = 0;
        pos += 4 * atlas->tex.w * ((atlas->tex.h - dim + 1) + i);
        pos -= 4 * dim;
        memset(&atlas->tex.data[pos], 255, dim * 4 * sizeof(char));
    }
}

/* TEXTURE INITIALIZATION */
void bs_initTexture(bs_Texture *texture, int w, int h, unsigned char *data) {
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    curr_texture = texture;
    curr_texture->w = w;
    curr_texture->h = h;
    curr_texture->data = data;
}

void bs_setTextureSettings(int min_filter, int mag_filter) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

void bs_genTextureMipmaps() {
    glGenerateMipmap(GL_TEXTURE_2D);
}

bs_Atlas *bs_createTextureAtlas(int w, int h, int max_textures) {
    atlases = realloc(atlases, sizeof(bs_Atlas) * (atlas_count+1));
    bs_Atlas *atlas = &atlases[atlas_count];

    atlas->tex.data = calloc(w * h * 4, sizeof(char));
    atlas->textures = malloc(sizeof(bs_TextureSlice) * max_textures);
    atlas->tex.w = w;
    atlas->tex.h = h;
    atlas->tex_count = 0;

    // White square can be used as default texture
    // allows multiplication of textures with color-only primitives
    bs_createWhiteSquare(BS_ATLAS_SIZE / 128, atlas);

    atlas_count++;

    return atlas;
}

void bs_pushAtlas(bs_Atlas *atlas) {
    bs_setOffsets(atlas->tex.w, atlas->tex.h, atlas);
    bs_appendToAtlas(atlas->tex.data, atlas->tex.w, atlas->tex.h, atlas);

    bs_initTexture(&atlas->tex, atlas->tex.w, atlas->tex.h, NULL);
    bs_setTextureSettings(BS_NEAREST_MIPMAP_LINEAR, BS_NEAREST);
    bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UINT);
    bs_genTextureMipmaps();
}

bs_TextureSlice *bs_loadTexture(char *path, int frames) {
    bs_Atlas *std_atlas = bs_getStdAtlas();

    bs_TextureSlice *tex = std_atlas->textures + std_atlas->tex_count;
    unsigned char *data;

    int success = lodepng_decode32_file(&data, &tex->w, &tex->h, path);

    if(success != 0) {
        printf("Texture wasn't loaded: %d\n", success);
    }

    bs_splitTexture(data, tex->w, tex->h, frames, &std_atlas->tex_count, &std_atlas->textures);

    std_atlas->tex_count += frames;

    return tex;
}

void bs_selectTexture(bs_TextureSlice *texture) {
    curr_texture_slice = texture;
}

void bs_saveAtlasToFile(bs_Atlas *atlas, char *name) {
    lodepng_encode32_file(name, atlas->tex.data, atlas->tex.w, atlas->tex.h);
}

void bs_freeAtlasData(bs_Atlas *atlas) {
    free(atlas->tex.data);
}

void bs_selectAtlas(bs_Atlas *atlas) {
    // glActiveTexture(GL_TEXTURE0 + atlas->id);
    glBindTexture(GL_TEXTURE_2D, atlas->tex.id);
}

bs_TextureSlice *bs_getSelectedTexture() {
    return curr_texture_slice;
}