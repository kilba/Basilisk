#ifndef BS_TEXTURES_H
#define BS_TEXTURES_H

typedef struct {
    unsigned int w, h;
    unsigned int x, y;
    float tex_x, tex_y;
    float tex_wx, tex_hy;
    unsigned char *data;
} bs_TextureSlice;

typedef struct {
    int w, h;
    unsigned int id;
    unsigned char *data;
} bs_Texture;

typedef struct {
    bs_Texture tex;
    bs_TextureSlice *textures;

    int tex_count;
} bs_Atlas;

/* --- TEXTURES --- */
bs_Atlas *bs_createTextureAtlas(int width, int height, int max_textures);
bs_TextureSlice *bs_loadTexture(char *path, int frames);
bs_TextureSlice *bs_getSelectedTexture();
void bs_selectTexture(bs_TextureSlice *texture);
void bs_pushAtlas(bs_Atlas *atlas);
void bs_saveAtlasToFile(bs_Atlas *atlas, char *name);
void bs_freeAtlasData(bs_Atlas *atlas);
void bs_selectAtlas(bs_Atlas *atlas);

void bs_initTexture(bs_Texture *texture, int w, int h, unsigned char *data);
void bs_setTextureSettings(int min_filter, int mag_filter);
void bs_pushTexture(int internal_format, int format, int type);
void bs_genTextureMipmaps();

#endif /* BS_TEXTURES_H */