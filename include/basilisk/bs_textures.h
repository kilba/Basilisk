#ifndef BS_TEXTURES_H
#define BS_TEXTURES_H

#include <bs_types.h>

void bs_saveTexture32(const char *name, unsigned char *data, int w, int h);
void bs_texture(bs_Texture *texture, bs_ivec2 dim, int type);
void bs_textureDataRaw(unsigned char *data);
bs_U32 bs_textureDataFile(const char *path, bool update_dimensions);
void bs_loadTex2D(bs_Texture *tex, char *path);
void bs_textureSettings(int min_filter, int mag_filter);
void bs_pushTexture(int internal_format, int format, int type);
void bs_textureMipmaps();
void bs_textureMinMag(int min_filter, int mag_filter);
void bs_textureWrap(int mode, int value);

void bs_textureSplit(int frame_count);
void bs_textureSplitVert(int frame_count);

void bs_setTexture(bs_Texture *texture);
void bs_selectTextureTarget(bs_Texture *texture, int tex_unit, int target);
void bs_selectTexture(bs_Texture *texture, int tex_unit);
bs_Texture *bs_selectedTexture();

void bs_texturePNG(bs_Texture *texture, char *path);
void bs_textureLinPNG(bs_Texture *texture, char *path);

void bs_depthCube(bs_Texture *texture, int dim);
void bs_depthCubeLin(bs_Texture *texture, int dim);
void bs_textureCube(bs_Texture *texture, int dim, char *paths[6]);
void bs_textureCubeLin(bs_Texture *texture, int dim, char *paths[6]);

void bs_textureArray(bs_Texture *tex, bs_ivec2 max_dim, int num_textures);
bs_U32 bs_textureArrayAppendPNG(const char *path);
bs_U32 bs_textureArrayAppendPNGSheet(const char *path, int frames);

void bs_linearFiltering();
void bs_nearestFiltering();

void bs_depthStencil(bs_Texture *texture, bs_ivec2 dim);
void bs_depth(bs_Texture *texture, bs_ivec2 dim);
void bs_textureRGB(bs_Texture *texture, bs_ivec2 dim);
void bs_textureRGB16f(bs_Texture *texture, bs_ivec2 dim);
void bs_textureRGB32f(bs_Texture *texture, bs_ivec2 dim);
void bs_textureRGBA(bs_Texture *texture, bs_ivec2 dim);
void bs_textureRGBA16f(bs_Texture *texture, bs_ivec2 dim);
void bs_textureRGBA32f(bs_Texture *texture, bs_ivec2 dim);
void bs_texture_11_11_10(bs_Texture *texture, bs_ivec2 dim);
void bs_textureR16U(bs_Texture *texture, bs_ivec2 dim);

#endif /* BS_TEXTURES_H */
