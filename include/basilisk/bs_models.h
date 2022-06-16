#ifndef BS_MODELS_H
#define BS_MODELS_H
#include <bs_core.h>

typedef struct {
	bs_vec3 *vertices;
	int vertex_count;
	int *indices;
	int index_count;
} bs_Model;

bs_Model bs_loadModel();

#endif /* BS_MODELS_H */