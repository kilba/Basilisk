#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.h>
#include <bs_models.h>
#include <bs_core.h>

bs_Model bs_loadModel() {
	bs_Model model;

	cgltf_options options = {0};
	cgltf_data* data = NULL;
	cgltf_parse_file(&options, "resources/untitled.gltf", &data);
	cgltf_load_buffers(&options, data, "resources/untitled.bin");
	int num_floats = cgltf_accessor_unpack_floats(&data->accessors[0], NULL, 0);
	int num_indices = cgltf_accessor_unpack_floats(&data->accessors[3], NULL, 0);

	float *verticesfloat = malloc(num_floats * sizeof(float));
	model.indices = malloc(num_indices * sizeof(int));
	model.vertices = malloc(num_floats * sizeof(float));
	model.vertex_count = num_floats / 3;
	model.index_count = num_indices;

	cgltf_accessor_unpack_floats(&data->accessors[0], verticesfloat, num_floats);
	
	for(int i = 0; i < num_floats; i += 3) {
		model.vertices[i / 3].x = verticesfloat[i+0] * 10.0;
		model.vertices[i / 3].y = verticesfloat[i+1] * 10.0;
		model.vertices[i / 3].z = verticesfloat[i+2] * 10.0;
	}

	for(int i = 0; i < num_indices; i++) {
		cgltf_uint outv;
		cgltf_accessor_read_uint(&data->accessors[3], i, &outv, 1);
		model.indices[i] = outv;
	}


	return model;
}