#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.h>

#include <stdint.h>

#include <bs_models.h>
#include <bs_core.h>
#include <bs_textures.h>
#include <bs_file_mgmt.h>
#include <bs_math.h>

int64_t curr_tex_ptr = 0;
int attrib_offset = 0;

/* --- VERTEX LOADING --- */
void bs_readPositionVertices(int accessor_index, bs_Prim *prim, cgltf_data *data) {
	int num_floats = cgltf_accessor_unpack_floats(&data->accessors[accessor_index], NULL, 0);
	float *vertices = malloc(num_floats * sizeof(float));
	cgltf_accessor_unpack_floats(&data->accessors[accessor_index], vertices, num_floats);

	for(int i = 0; i < num_floats; i+=3) {
		vertices[i+0] *= 10.0;
		vertices[i+1] *= 10.0;
		vertices[i+2] *= 10.0;

		memcpy(&prim->vertices[i/3].position, &vertices[i], 3 * sizeof(float));
	}

	free(vertices);
	vertices = NULL;

}

void bs_readNormalVertices(int accessor_index, bs_Prim *prim, cgltf_data *data) {
	int num_floats = cgltf_accessor_unpack_floats(&data->accessors[accessor_index], NULL, 0);
	float *normals = malloc(num_floats * sizeof(float));
	cgltf_accessor_unpack_floats(&data->accessors[accessor_index], normals, num_floats);

	for(int i = 0; i < num_floats; i+=3) {
		memcpy(&prim->vertices[i/3].normal, &normals[i], 3 * sizeof(float));
	}

	free(normals);
	normals = NULL;
}

void bs_readTexCoordVertices(int accessor_index, bs_Prim *prim, cgltf_data *data) {
	int num_floats = cgltf_accessor_unpack_floats(&data->accessors[accessor_index], NULL, 0);
	float *tex_coords = malloc(num_floats * sizeof(float));
	cgltf_accessor_unpack_floats(&data->accessors[accessor_index], tex_coords, num_floats);

	for(int i = 0; i < num_floats; i+=2) {
		memcpy(&prim->vertices[i/2].tex_coord, &tex_coords[i], 2 * sizeof(float));
	}

	if(prim->material.tex != NULL) {
		float x_range = (float)prim->material.tex->w / BS_ATLAS_SIZE;
		float y_range = (float)prim->material.tex->h / BS_ATLAS_SIZE;

		for(int i = 0; i < num_floats; i+=2) {
			prim->vertices[i/2].tex_coord.x = bs_fMap(prim->vertices[i/2].tex_coord.x, 0.0, 1.0, 0.0, x_range);
			prim->vertices[i/2].tex_coord.y = bs_fMap(prim->vertices[i/2].tex_coord.y, 0.0, 1.0, 0.0, y_range);
		}
	}

	free(tex_coords);
	tex_coords = NULL;
}


void bs_loadMaterial(bs_Model *model, cgltf_primitive *c_prim, bs_Prim *prim) {
	cgltf_material *c_mat = c_prim->material;
	cgltf_float *mat_color = c_mat->pbr_metallic_roughness.base_color_factor;
	bs_Material *mat = &prim->material;

	mat->base_color.r = mat_color[0] * 255;
	mat->base_color.g = mat_color[1] * 255;
	mat->base_color.b = mat_color[2] * 255;
	mat->base_color.a = mat_color[3] * 255;

	// If the primitve has a texture
	if(c_mat->pbr_metallic_roughness.base_color_texture.texture != NULL) {
		int id = (int64_t)c_mat->pbr_metallic_roughness.base_color_texture.texture->image;
		id -= curr_tex_ptr;
		id /= sizeof(cgltf_image);

		mat->tex = model->textures[id];
	}
}

void bs_loadPrim(cgltf_data *data, bs_Mesh *mesh, bs_Model *model, int mesh_index, int prim_index) {
	cgltf_mesh *c_mesh = &data->meshes[mesh_index];
	bs_Prim *prim = &mesh->prims[prim_index];
	prim->material.tex = NULL;

	bool has_unique_indices = true;

	if(attrib_offset != 0) {
		// If the address of the previous primitives indices is less or equal to the current one,
		// the primitive does not have unique indices
		has_unique_indices = !(c_mesh->primitives[prim_index].indices <= c_mesh->primitives[prim_index-1].indices);
	}

	int attrib_count = c_mesh->primitives[prim_index].attributes_count;
	int num_floats = cgltf_accessor_unpack_floats(&data->accessors[attrib_offset], NULL, 0);

	prim->vertices = malloc(num_floats * sizeof(bs_Vertex));
	prim->vertex_count = num_floats / 3;

	bs_loadMaterial(model, &c_mesh->primitives[prim_index], prim);

	// Read vertices
    for(int i = 0; i < attrib_count; i++) {
    	int type = c_mesh->primitives[prim_index].attributes[i].type;
    	switch(type) {
    		case cgltf_attribute_type_position:
    			bs_readPositionVertices(attrib_offset + i, prim, data); break;
			case cgltf_attribute_type_normal:
				bs_readNormalVertices(attrib_offset + i, prim, data); break;
			case cgltf_attribute_type_texcoord:
				bs_readTexCoordVertices(attrib_offset + i, prim, data); break;
    	}
    }

    // Read indices
	int num_indices = cgltf_accessor_unpack_floats(c_mesh->primitives[prim_index].indices, NULL, 0);
	prim->indices = malloc(num_indices * sizeof(int));
	prim->index_count = num_indices;
	for(int i = 0; i < num_indices; i++) {
		cgltf_uint outv;
		cgltf_accessor_read_uint(c_mesh->primitives[prim_index].indices, i, &outv, 1);
		prim->indices[i] = outv;
	}

	attrib_offset += (attrib_count + has_unique_indices);

	mesh->vertex_count += prim->vertex_count;
	model->vertex_count += prim->vertex_count;
	model->index_count += num_indices;
}

void bs_loadMesh(cgltf_data *data, bs_Model *model, int mesh_index) {
	cgltf_mesh *c_mesh = &data->meshes[mesh_index];
	cgltf_node *node = &data->nodes[mesh_index];

	memcpy(&model->meshes[mesh_index].pos, node->translation, sizeof(bs_vec3));
	memcpy(&model->meshes[mesh_index].rot, node->rotation, sizeof(bs_vec4));
	memcpy(&model->meshes[mesh_index].sca, node->scale, sizeof(bs_vec4));

	model->meshes[mesh_index].prims = malloc(c_mesh->primitives_count * sizeof(bs_Prim));
	model->meshes[mesh_index].prim_count = c_mesh->primitives_count;
	for(int i = 0; i < c_mesh->primitives_count; i++) {
		bs_loadPrim(data, &model->meshes[mesh_index], model, mesh_index, i);
	}
}

void bs_loadModelTextures(cgltf_data* data, bs_Model *model) {
	bs_Tex2D *images[data->images_count];
	int64_t ids[data->textures_count];

	for(int i = 0; i < data->textures_count; i++) {
		// Getting the pointers to all images in the form of a 64 bit int
		ids[i] = (int64_t)data->textures[i].image;

		char texture_path[256] = "resources/models/textures/";
		strcat(texture_path, data->images[i].name);
		strcat(texture_path, ".png");
		images[i] = bs_loadTexture(texture_path, 1);
	}

 	curr_tex_ptr = ids[0];
 	model->textures = malloc(data->textures_count * sizeof(bs_Tex2D *));

 	printf("to\n");

	for(int i = 0; i < data->textures_count; i++) {
		ids[i] -= curr_tex_ptr;
		// Divide by sizeof image so that stride between indices in ids is at most 1
		ids[i] /= sizeof(cgltf_image);
		model->textures[i] = images[ids[i]];
	}
}

void bs_loadModel(char *model_path, char *texture_folder_path, bs_Model *model) {
	cgltf_options options = {0};
	cgltf_data* data = NULL;

	// Get path to GLTF binary data
	char bin_path[256];
	int path_len = strlen(model_path);
	strncpy(bin_path, model_path, path_len-4);
	strcat(bin_path, "bin");

	// Load the GLTF json and binary data
	cgltf_parse_file(&options, model_path, &data);
	cgltf_load_buffers(&options, data, bin_path);

	int mesh_count = data->meshes_count;

	model->meshes = malloc(mesh_count * sizeof(bs_Mesh));
	model->mesh_count = mesh_count;
	model->vertex_count = 0;
	model->index_count = 0;

	if(data->textures_count != 0) {
		bs_loadModelTextures(data, model);
	}

	for(int i = 0; i < mesh_count; i++) {
		bs_loadMesh(data, model, i);
	}

	// for(int i = 0; i < model->meshes[0].prim_count; i++) {
	// 	for(int j = 0; j < model->meshes[0].prims[0].vertex_count; j++) {
	// 		model->meshes[0].prims[i].vertices[j].tex_coord.x = bs_fMap(model->meshes[0].prims[i].vertices[j].tex_coord.x, 0.0, 1.0, 0.0, model->meshes[0].tex->w / 1024.0);
	// 		model->meshes[0].prims[i].vertices[j].tex_coord.y = bs_fMap(model->meshes[0].prims[i].vertices[j].tex_coord.y, 0.0, 1.0, 0.0, model->meshes[0].tex->h / 1024.0);
	// 	}
	// }

	// for(int i = 0; i < model->meshes[0].vertex_count; i++) {
	// 	printf("%f\n", model->meshes[0].prims[0].vertices[i].tex_coord.x);
	// 	printf("%f\n", model->meshes[0].vertices[i].tex_coord.x);
	// 	printf("\n");
	// }
}