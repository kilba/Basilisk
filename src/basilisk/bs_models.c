#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.h>
#include <bs_models.h>
#include <bs_core.h>

void bs_readPositionVertices(int accessor_index, bs_Mesh *mesh, cgltf_data *data) {
	printf("Reading positions...\n");
	int num_floats = cgltf_accessor_unpack_floats(&data->accessors[accessor_index], NULL, 0);
	printf("%d\n", num_floats);
	float *vertices = malloc(num_floats * sizeof(float));
	cgltf_accessor_unpack_floats(&data->accessors[accessor_index], vertices, num_floats);

	for(int i = 0; i < num_floats; i+=3) {
		vertices[i+0] *= 10.0;
		vertices[i+1] *= 10.0;
		vertices[i+2] *= 10.0;

		memcpy(&mesh->vertices[i/3].position, &vertices[i], 3 * sizeof(float));
	}

	free(vertices);
	vertices = NULL;
}

void bs_readNormalVertices(int accessor_index, bs_Mesh *mesh, cgltf_data *data) {
	printf("Reading normals...\n");
	int num_floats = cgltf_accessor_unpack_floats(&data->accessors[accessor_index], NULL, 0);
	float *normals = malloc(num_floats * sizeof(float));
	cgltf_accessor_unpack_floats(&data->accessors[accessor_index], normals, num_floats);

	for(int i = 0; i < num_floats; i+=3) {
		memcpy(&mesh->vertices[i/3].normal, &normals[i], 3 * sizeof(float));
	}

	free(normals);
	normals = NULL;
}

void bs_readTexCoordVertices(int accessor_index, bs_Mesh *mesh, cgltf_data *data) {
	printf("Reading texture coords...\n");
	int num_floats = cgltf_accessor_unpack_floats(&data->accessors[accessor_index], NULL, 0);
	float *tex_coords = malloc(num_floats * sizeof(float));
	cgltf_accessor_unpack_floats(&data->accessors[accessor_index], tex_coords, num_floats);

	for(int i = 0; i < num_floats; i+=2) {
		memcpy(&mesh->vertices[i/2].tex_coord, &tex_coords[i], 2 * sizeof(float));
	}

	free(tex_coords);
	tex_coords = NULL;
}

void bs_readColorVertices(int accessor_index, bs_Mesh *mesh, cgltf_data *data) {
	printf("Reading colors...\n");
}

void bs_loadMesh(cgltf_data *data, bs_Model *model, int mesh_index) {
	cgltf_mesh *mesh = &data->meshes[mesh_index];

	int attrib_count = mesh->primitives[0].attributes_count;
	int accessor_index = mesh_index * (attrib_count+1);
	int num_floats = cgltf_accessor_unpack_floats(&data->accessors[accessor_index+0], NULL, 0);
	printf("accessor_index: %d\n", accessor_index);
	printf("num_floats: %d\n", num_floats);

	model->meshes[mesh_index].vertices = malloc(num_floats * sizeof(bs_Vertex));
	model->meshes[mesh_index].vertex_count = num_floats / 3;
	model->vertex_count += model->meshes[mesh_index].vertex_count;

    for(int i = 0; i < attrib_count; i++) {
    	int type = mesh->primitives[0].attributes[i].type;
    	switch(type) {
    		case cgltf_attribute_type_position:
    			bs_readPositionVertices(accessor_index + i, &model->meshes[mesh_index], data); break;
			case cgltf_attribute_type_normal:
				bs_readNormalVertices(accessor_index + i, &model->meshes[mesh_index], data); break;
			case cgltf_attribute_type_texcoord:
				bs_readTexCoordVertices(accessor_index + i, &model->meshes[mesh_index], data); break;
			case cgltf_attribute_type_color:
				bs_readColorVertices(accessor_index + i, &model->meshes[mesh_index], data); break;
    	}
    }

    // TODO: Extract to function
    // Read indices
	int num_indices = cgltf_accessor_unpack_floats(&data->accessors[accessor_index+3], NULL, 0);
	model->meshes[mesh_index].indices = malloc(num_indices * sizeof(int));
	model->meshes[mesh_index].index_count = num_indices;
	model->index_count += num_indices;
	for(int i = 0; i < num_indices; i++) {
		cgltf_uint outv;
		cgltf_accessor_read_uint(&data->accessors[accessor_index+3], i, &outv, 1);
		model->meshes[mesh_index].indices[i] = outv;
	}
}

void bs_loadModel(char *path, bs_Model *model) {
	cgltf_options options = {0};
	cgltf_data* data = NULL;

	// Get path to GLTF binary data
	char bin_path[256];
	int path_len = strlen(path);
	strncpy(bin_path, path, path_len-4);
	strcat(bin_path, "bin");

	// Load the GLTF json and binary data
	cgltf_parse_file(&options, path, &data);
	cgltf_load_buffers(&options, data, bin_path);
	printf(data->json);

	int mesh_count = data->meshes_count;

	model->meshes = malloc(mesh_count * sizeof(bs_Mesh));
	model->mesh_count = mesh_count;
	model->vertex_count = 0;
	model->index_count = 0;
	for(int i = 0; i < mesh_count; i++) {
		bs_loadMesh(data, model, i);
	}
}