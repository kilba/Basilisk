#ifndef BS_TEXTURES_H
#define BS_TEXTURES_H

#include <bs_types.h>

/* --- TEXTURES --- */
void bs_texture(bs_Texture *texture, bs_ivec2 dim, int type);
void bs_textureDataRaw(unsigned char *data);
bs_U32 bs_textureDataFile(char *path, bool update_dimensions);
void bs_loadTex2D(bs_Texture *tex, char *path);
void bs_textureSettings(int min_filter, int mag_filter);
void bs_pushTexture(int internal_format, int format, int type);
void bs_textureMipmaps();
void bs_textureMinMag(int min_filter, int mag_filter);

void bs_texSplit(int frame_count);
void bs_texSplitVert(int frame_count);

void bs_selectTextureTarget(bs_Texture *texture, int tex_unit, int target);
void bs_selectTexture(bs_Texture *texture, int tex_unit);
bs_Texture *bs_selectedTexture();

void bs_depthStencil(bs_Texture *texture, bs_ivec2 dim);

void bs_depth(bs_Texture *texture, bs_ivec2 dim);
void bs_depthLin(bs_Texture *texture, bs_ivec2 dim);

void bs_textureRGBA(bs_Texture *texture, bs_ivec2 dim);
void bs_textureLinRGBA(bs_Texture *texture, bs_ivec2 dim);

void bs_texturePNG(bs_Texture *texture, char *path);
void bs_textureLinPNG(bs_Texture *texture, char *path);

void bs_depthCube(bs_Texture *texture, int dim);
void bs_depthCubeLin(bs_Texture *texture, int dim);
void bs_textureCube(bs_Texture *texture, int dim, char *paths[6]);
void bs_textureCubeLin(bs_Texture *texture, int dim, char *paths[6]);

void bs_textureArray(bs_Texture *tex, bs_ivec2 max_dim, int num_textures);
bs_U32 bs_textureArrayAppendPNG(char *path);

void bs_textureRGBA16f(bs_Texture *tex, bs_ivec2 dim);
void bs_textureLinRGBA16f(bs_Texture *tex, bs_ivec2 dim);
void bs_textureRGBA32f(bs_Texture *tex, bs_ivec2 dim);
void bs_textureLinRGBA32f(bs_Texture *tex, bs_ivec2 dim);

void bs_texture_11_11_10(bs_Texture *tex, bs_ivec2 dim);
void bs_textureLin_11_11_10(bs_Texture *tex, bs_ivec2 dim);

#endif /* BS_TEXTURES_H */
