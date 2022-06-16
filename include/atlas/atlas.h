#ifndef ATLAS_H
#define ATLAS_H

typedef struct {
    unsigned int w, h;
    unsigned int x, y;
    float tex_x, tex_y;
    float tex_wx, tex_hy;
    unsigned char *data;
} atl_Texture;

atl_Texture *atl_loadTexture(char *path, int frames, int *curr_tex_count, atl_Texture **textures);
void atl_setOffsets(int width, int height, int *curr_tex_count, atl_Texture **textures);
void atl_appendToAtlas(unsigned char *atlas_data, int width, int height, int *curr_tex_count, atl_Texture **textures);

#endif /* ATLAS_H */