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
bs_Tex2D *curr_texture;

void bs_splitTexture(unsigned char *data, int w, int h, int frames, int *curr_tex_count, bs_Tex2D **textures) {
    int slice_width = w / frames; // TODO: Check if not an int

    // For each frame
    for(int i = 0; i < frames; i++) {
        bs_Tex2D *tex = (*textures) + *curr_tex_count + i;
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
    bs_Tex2D *tex = atlas->textures;

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
        bs_Tex2D *tex = &atlas->textures[i];
        cappend_append(atlas_data, width, height, tex->data, tex->w, tex->h, tex->x, tex->y);
        free(tex->data);
        tex->data = NULL;
    }
}

bs_Atlas *bs_createTextureAtlas(int width, int height, int max_textures) {
    atlases = realloc(atlases, sizeof(bs_Atlas) * (atlas_count+1));
    bs_Atlas *atlas = &atlases[atlas_count];

    atlas->data = calloc(width * height * 4, sizeof(char));
    atlas->textures = malloc(sizeof(bs_Tex2D) * max_textures);
    atlas->w = width;
    atlas->h = height;
    atlas->id = atlas_count;
    atlas->tex_count = 0;

    atlas_count++;

    return atlas;
}

void bs_pushAtlas(bs_Atlas *atlas) {
    bs_setOffsets(atlas->w, atlas->h, atlas);
    bs_appendToAtlas(atlas->data, atlas->w, atlas->h, atlas);

    glGenTextures(1, &atlas->tex_id);

    glActiveTexture(GL_TEXTURE0 + atlas->id);
    glBindTexture(GL_TEXTURE_2D, atlas->tex_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas->w, atlas->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlas->data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

bs_Tex2D *bs_loadTexture(char *path, int frames, bs_Atlas *atlas) {
    bs_Tex2D *tex = atlas->textures + atlas->tex_count;
    unsigned char *data;

    lodepng_decode32_file(&data, &tex->w, &tex->h, path);
    bs_splitTexture(data, tex->w, tex->h, frames, &atlas->tex_count, &atlas->textures);

    atlas->tex_count += frames;

    return tex;
}

void bs_selectTexture(bs_Tex2D *texture) {
    curr_texture = texture;
}

void bs_saveAtlasToFile(bs_Atlas *atlas, char *name) {
    lodepng_encode32_file(name, atlas->data, atlas->w, atlas->h);
}

void bs_freeAtlasData(bs_Atlas *atlas) {
    free(atlas->data);
}

void bs_selectAtlas(bs_Atlas *atlas) {
    // glActiveTexture(GL_TEXTURE0 + atlas->id);
    glBindTexture(GL_TEXTURE_2D, atlas->tex_id);
}

bs_Tex2D *bs_getSelectedTexture() {
    return curr_texture;
}