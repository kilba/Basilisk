#ifndef BS_TEXTURES_H
#define BS_TEXTURES_H

#include <bs_types.h>

/* --- TEXTURES --- */
bs_Atlas *bs_createTextureAtlas(int width, int height, int max_textures);
bs_AtlasSlice *bs_loadTexture(char *path, int frames);
bs_AtlasSlice *bs_getSelectedTexture();
void bs_selectTexture(bs_Tex2D *texture, int tex_unit);
void bs_pushAtlas(bs_Atlas *atlas);
void bs_saveAtlasToFile(bs_Atlas *atlas, char *name);
void bs_freeAtlasData(bs_Atlas *atlas);

void bs_initTexture(bs_Tex2D *texture, int w, int h, unsigned char *data);
void bs_setTextureSettings(int min_filter, int mag_filter);
void bs_pushTexture(int internal_format, int format, int type);
void bs_genTextureMipmaps();

#endif /* BS_TEXTURES_H */