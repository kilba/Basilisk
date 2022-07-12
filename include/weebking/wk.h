#ifndef WK_H
#define WK_H

#include <bs_core.h>
#include <bs_shaders.h>

typedef struct {
	bs_Batch   batch;
	bs_Model  *model;
	bs_Shader *shader;
} MovObj;

typedef struct {
	bs_Batch  batch;
	bs_Model *model;
} EnvObj;

#endif /* WK_H */