#ifndef RESOURCES_H
#define RESOURCES_H

#include <bs_core.h>

typedef enum {
	MODEL_PLAYER = 0,
	MODEL_ENV0,
} Model_IDS;

typedef enum {
	SHADER_PLAYER = 0,
} Shader_IDS;

typedef enum {
	TEX_PLAYER = 0,
} Tex_IDS;

struct Resources {
	bs_Model  models[16];
	bs_Shader shaders[16];
	bs_Tex2D  textures[16];

	int mov_obj_count;
} resources;

void loadResources();

#endif /* RESOURCES_H */