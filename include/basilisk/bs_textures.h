#ifndef BS_TEXTURES_H
#define BS_TEXTURES_H

typedef struct {
    unsigned int w, h;
    unsigned int x, y;
    float tex_x, tex_y;
    float tex_wx, tex_hy;
    unsigned char *data;
} bs_Tex2D;

typedef struct {
    int w, h;
    int id;
    unsigned int tex_id;
    unsigned char *data;

    int tex_count;
    bs_Tex2D *textures;
} bs_Atlas;

/* --- TEXTURES --- */
bs_Atlas *bs_createTextureAtlas(int width, int height, int max_textures);
bs_Tex2D *bs_loadTexture(char *path, int frames);
void bs_selectTexture(bs_Tex2D *texture);
void bs_pushAtlas(bs_Atlas *atlas);
void bs_saveAtlasToFile(bs_Atlas *atlas, char *name);
void bs_freeAtlasData(bs_Atlas *atlas);
void bs_selectAtlas(bs_Atlas *atlas);
bs_Tex2D *bs_getSelectedTexture();

#endif /* BS_TEXTURES_H */