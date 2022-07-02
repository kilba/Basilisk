#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.h>
#include <cglm/cglm.h>

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
	int num_comps = cgltf_num_components(data->accessors[accessor_index].type);

	for(int i = 0; i < num_floats / num_comps; i++) {
		cgltf_accessor_read_float(&data->accessors[accessor_index], i, &prim->vertices[i].position.x, num_comps);
	}
}

void bs_readNormalVertices(int accessor_index, bs_Prim *prim, cgltf_data *data) {
	int num_floats = cgltf_accessor_unpack_floats(&data->accessors[accessor_index], NULL, 0);
	int num_comps = cgltf_num_components(data->accessors[accessor_index].type);

	for(int i = 0; i < num_floats / num_comps; i++) {
		cgltf_accessor_read_float(&data->accessors[accessor_index], i, &prim->vertices[i].normal.x, num_comps);
	}
}

void bs_readTexCoordVertices(int accessor_index, bs_Prim *prim, cgltf_data *data) {
	int num_floats = cgltf_accessor_unpack_floats(&data->accessors[accessor_index], NULL, 0);
	int num_comps = cgltf_num_components(data->accessors[accessor_index].type);

	for(int i = 0; i < num_floats / num_comps; i++) {
		cgltf_accessor_read_float(&data->accessors[accessor_index], i, &prim->vertices[i].tex_coord.x, num_comps);
	}

	if(prim->material.tex != NULL) {
		float x_range = (float)prim->material.tex->w / BS_ATLAS_SIZE;
		float y_range = (float)prim->material.tex->h / BS_ATLAS_SIZE;

		for(int i = 0; i < num_floats; i+=2) {
			prim->vertices[i/2].tex_coord.x = bs_fMap(prim->vertices[i/2].tex_coord.x, 0.0, 1.0, 0.0, x_range);
			prim->vertices[i/2].tex_coord.y = bs_fMap(prim->vertices[i/2].tex_coord.y, 0.0, 1.0, 0.0, y_range);
		}
	}
}

void bs_readJointIndices(int accessor_index, bs_Prim *prim, cgltf_data *data) {
	int num_ints = cgltf_accessor_unpack_floats(&data->accessors[accessor_index], NULL, 0);
	int num_comps = cgltf_num_components(data->accessors[accessor_index].type);

	for(int i = 0; i < num_ints / num_comps; i++) {
		unsigned int xyzw[4];

		cgltf_accessor_read_uint(&data->accessors[accessor_index], i, xyzw, num_comps);
		prim->vertices[i].bone_ids.x = xyzw[0];
		prim->vertices[i].bone_ids.y = xyzw[1];
		prim->vertices[i].bone_ids.z = xyzw[2];
		prim->vertices[i].bone_ids.w = xyzw[3];

		// printf("%d, ", prim->vertices[i].bone_ids.x);
		// printf("%d, ", prim->vertices[i].bone_ids.y);
		// printf("%d, ", prim->vertices[i].bone_ids.z);
		// printf("%d, ", prim->vertices[i].bone_ids.w);
		// printf("\n");
	}
}

void bs_readWeights(int accessor_index, bs_Prim *prim, cgltf_data *data) {
	int num_floats = cgltf_accessor_unpack_floats(&data->accessors[accessor_index], NULL, 0);
	int num_comps = cgltf_num_components(data->accessors[accessor_index].type);

	for(int i = 0; i < num_floats / num_comps; i++) {
		cgltf_accessor_read_float(&data->accessors[accessor_index], i, &prim->vertices[i].weights.x, num_comps);
	}
}

void bs_loadMaterial(bs_Model *model, cgltf_primitive *c_prim, bs_Prim *prim) {
	cgltf_material *c_mat = c_prim->material;
	bs_Material *mat = &prim->material;

	if(c_mat == NULL) {
		mat->base_color.r = 255;
		mat->base_color.g = 255;
		mat->base_color.b = 255;
		mat->base_color.a = 255;		
		return;
	}

	cgltf_float *mat_color = c_mat->pbr_metallic_roughness.base_color_factor;

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

	int attrib_count = c_mesh->primitives[prim_index].attributes_count;
	int num_floats = cgltf_accessor_unpack_floats(&data->accessors[c_mesh->primitives[prim_index].attributes[0].index], NULL, 0);

	prim->vertices = malloc(num_floats * sizeof(bs_Vertex));
	prim->vertex_count = num_floats / 3;

	bs_loadMaterial(model, &c_mesh->primitives[prim_index], prim);

	// Read vertices
    for(int i = 0; i < attrib_count; i++) {
    	int index = c_mesh->primitives[prim_index].attributes[i].index;
    	int type = c_mesh->primitives[prim_index].attributes[i].type;

    	switch(type) {
    		case cgltf_attribute_type_position:
    			bs_readPositionVertices(index, prim, data); break;
			case cgltf_attribute_type_normal:
				bs_readNormalVertices(index, prim, data); break;
			case cgltf_attribute_type_texcoord:
				bs_readTexCoordVertices(index, prim, data); break;
			case cgltf_attribute_type_joints:
				bs_readJointIndices(index, prim, data); break;
			case cgltf_attribute_type_weights:
				bs_readWeights(index, prim, data); break;
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

	attrib_offset = c_mesh->primitives[prim_index].index_id + 1;

	mesh->vertex_count += prim->vertex_count;
	model->vertex_count += prim->vertex_count;
	model->index_count += num_indices;
}

void bs_printMat4(bs_mat4 matrix) {
	for(int y = 0; y < 4; y++) {
		for(int x = 0; x < 4; x++) {
			printf("%f, ", matrix[x][y]);
		}
		printf("\n");
	}
	printf("\n");
}

void bs_loadJoints(cgltf_data *data, bs_Mesh *mesh, cgltf_mesh *c_mesh) {
	cgltf_skin *skin = c_mesh->node->skin;
	mesh->joints = malloc(skin->joints_count * sizeof(bs_Joint));
	mesh->joint_count = skin->joints_count;

	for(int i = 0; i < skin->joints_count; i++) {
		cgltf_node *c_joint = skin->joints[i];
		cgltf_accessor_read_float(skin->inverse_bind_matrices, i, (float*)&mesh->joints[i].bind_matrix_inv, 16);

		mat4 local = GLM_MAT4_IDENTITY_INIT;
		memcpy(mesh->joints[i].mat, local, 16 * sizeof(float));

		mesh->joints[i].name = skin->joints[i]->name;
		c_joint->id = i;
	}

	for(int i = 0; i < skin->joints_count; i++) {
		cgltf_node *c_joint = skin->joints[i];
		int parent_id = skin->joints[i]->parent->id;

		mesh->joints[i].parent = &mesh->joints[parent_id];
	}

	mat4 local_o = GLM_MAT4_IDENTITY_INIT;
	mat4 bind_inv_o;
	mat4 bind_o;
	memcpy(bind_inv_o, mesh->joints[7].bind_matrix_inv, 16 * sizeof(float));
	glm_mat4_inv(bind_inv_o, bind_o);

	mat4 test = GLM_MAT4_IDENTITY_INIT;
	// glm_quat_rotate(test, (versor){ 3.7, 0.0, 0.0, 1.0 }, test);
	glm_rotate(bind_o, 2.96705973, (vec3){ 1.0, 0.0, 0.0 });
	glm_mat4_mul(test, bind_inv_o, test);
	glm_mat4_mul(bind_o, test, bind_o);

	glm_mat4_mul(local_o, bind_o, local_o);
	// glm_mat4_mul(local_o, bind_inv_o, local_o);
	// glm_quat_rotate(local_o, (versor){ 0.8, 0.0, 0.0, 1.0 }, local_o);
	bs_printMat4(local_o);

	memcpy(mesh->joints[7].mat, local_o, 16 * sizeof(float));

	for(int i = 0; i < skin->joints_count; i++) {
		bs_Joint *joint = &mesh->joints[i];
		bs_Joint *parent = mesh->joints[i].parent;

		bs_mat4 joint_mat;
		bs_mat4 parent_mat;

		memcpy(joint_mat, joint->mat, 16 * sizeof(float));
		memcpy(parent_mat, parent->mat, 16 * sizeof(float));

		glm_mat4_mul(joint_mat, parent_mat, joint_mat);

		memcpy(joint->mat, joint_mat, 16 * sizeof(float));
	}
}

void bs_loadMesh(cgltf_data *data, bs_Model *model, int mesh_index) {
	cgltf_mesh *c_mesh = &data->meshes[mesh_index];
	cgltf_node *node = &data->nodes[mesh_index];

	memcpy(&model->meshes[mesh_index].pos, node->translation, sizeof(bs_vec3));
	memcpy(&model->meshes[mesh_index].rot, node->rotation, sizeof(bs_vec4));
	memcpy(&model->meshes[mesh_index].sca, node->scale, sizeof(bs_vec4));

	model->meshes[mesh_index].prims = malloc(c_mesh->primitives_count * sizeof(bs_Prim));
	model->meshes[mesh_index].prim_count = c_mesh->primitives_count;

	bs_loadJoints(data, &model->meshes[mesh_index], c_mesh);

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
	}
	for(int i = 0; i < data->images_count; i++) {
		char texture_path[256] = "resources/models/textures/";
		strcat(texture_path, data->images[i].name);
		strcat(texture_path, ".png");
		images[i] = bs_loadTexture(texture_path, 1);
	}

 	curr_tex_ptr = ids[0];
 	model->textures = malloc(data->textures_count * sizeof(bs_Tex2D *));

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
}