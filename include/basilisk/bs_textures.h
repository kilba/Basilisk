#ifndef BS_TEXTURES_H
#define BS_TEXTURES_H

#include <bs_types.h>

/* --- TEXTURES --- */
void bs_selectTexture(bs_Tex2D *texture, int tex_unit);
void bs_texture(bs_Tex2D *texture, int w, int h);
void bs_textureDataRaw(unsigned char *data);
void bs_textureDataFile(char *path, bool update_dimensions);
void bs_loadTex2D(bs_Tex2D *tex, char *path);
void bs_textureSettings(int min_filter, int mag_filter);
void bs_pushTexture(int internal_format, int format, int type);
void bs_textureMipmaps();

void bs_depth(bs_Tex2D *texture, int w, int h);
void bs_depthLin(bs_Tex2D *texture, int w, int h);

void bs_textureRGBA(bs_Tex2D *texture, int w, int h);
void bs_textureLinRGBA(bs_Tex2D *texture, int w, int h);

void bs_texturePNG(bs_Tex2D *texture, char *path);
void bs_textureLinPNG(bs_Tex2D *texture, char *path);

#endif /* BS_TEXTURES_H */
