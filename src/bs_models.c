#include <stdio.h>
#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.h>
#include <cglm/cglm.h>

#include <stdint.h>

#include <bs_mem.h>
#include <bs_core.h>
#include <bs_math.h>
#include <bs_models.h>
#include <bs_shaders.h>
#include <bs_textures.h>

bs_Joint identity_joint = { GLM_MAT4_IDENTITY_INIT };

typedef struct {
    int frame;
    bs_mat4 joints[];
} SSBO_Data;

bs_Anim *anims = NULL;
int anim_count = 0;
int allocated_anims = 0;

int64_t curr_tex_ptr = 0;
bs_I32 anim_ssbo = -1;
bs_U32 anim_offset = 0;
int attrib_offset = 0;

int load_settings = 0;

/* --- VERTEX LOADING --- */
void bs_readPositionVertices(int accessor_index, bs_Prim *prim, bs_Mesh *mesh, cgltf_data *data) {	
    int num_floats = cgltf_accessor_unpack_floats(&data->accessors[accessor_index], NULL, 0);
    int num_comps = cgltf_num_components(data->accessors[accessor_index].type);

    for(int i = 0; i < num_floats / num_comps; i++) {
	cgltf_accessor_read_float(&data->accessors[accessor_index], i, &prim->vertices[i].position.x, num_comps);
	vec3 *pos = (vec3*)&prim->vertices[i].position;
	glm_mat4_mulv3(mesh->mat, *pos, 1.0, *pos);
    }
}

void bs_readNormalVertices(int accessor_index, bs_Prim *prim, bs_Mesh *mesh, cgltf_data *data) {
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
	float x_range = (float)prim->material.tex->w / 1024.0;
	float y_range = (float)prim->material.tex->h / 1024.0;

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
	mat->col.r = 255;
	mat->col.g = 255;
	mat->col.b = 255;
	mat->col.a = 255;
	return;
    }

    cgltf_pbr_metallic_roughness *metallic = &c_mat->pbr_metallic_roughness;
    cgltf_float *mat_color = metallic->base_color_factor;

    mat->col.r = mat_color[0] * 255;
    mat->col.g = mat_color[1] * 255;
    mat->col.b = mat_color[2] * 255;
    mat->col.a = mat_color[3] * 255;

    // If the primitive has a texture
    if(metallic->base_color_texture.texture != NULL) {
	int id = (int64_t)metallic->base_color_texture.texture->image;
	id -= curr_tex_ptr;
	id /= sizeof(cgltf_image);

	mat->tex = &model->textures[0];
    }

    mat->metallic = metallic->metallic_factor;
}

void bs_readIndices(bs_Mesh *mesh, bs_Model *model, cgltf_mesh *c_mesh, int prim_index) {
    bs_Prim *prim = mesh->prims + prim_index;

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

void bs_readIndicesAdjacent() {
}

/* Temp */
typedef struct {
    bs_vec3 position;
    bs_vec2 tex_coord;
    bs_vec3 normal;
    bs_RGBA color;
    bs_ivec4 bone_ids;
    bs_vec4 weights;
    int unique_index;
} VertexDecl;

void bs_loadPrim(cgltf_data *data, bs_Mesh *mesh, bs_Model *model, int mesh_index, int prim_index) {
    cgltf_mesh *c_mesh = &data->meshes[mesh_index];
    bs_Prim *prim = &mesh->prims[prim_index];
    prim->material.tex = NULL;

    int attrib_count = c_mesh->primitives[prim_index].attributes_count;
    int num_floats = cgltf_accessor_unpack_floats(&data->accessors[c_mesh->primitives[prim_index].attributes[0].index], NULL, 0);

    prim->vertex_count = num_floats / 3;
    prim->vertices = malloc(prim->vertex_count * sizeof(VertexDecl));

    bs_loadMaterial(model, &c_mesh->primitives[prim_index], prim);

    // Read vertices
    for(int i = 0; i < attrib_count; i++) {
    	int index = c_mesh->primitives[prim_index].attributes[i].index;
    	int type = c_mesh->primitives[prim_index].attributes[i].type;

    	switch(type) {
	    case cgltf_attribute_type_position:
		bs_readPositionVertices(index, prim, mesh, data); break;
	    case cgltf_attribute_type_normal:
		bs_readNormalVertices(index, prim, mesh, data); break;
	    case cgltf_attribute_type_texcoord:
		bs_readTexCoordVertices(index, prim, data); break;
	    case cgltf_attribute_type_joints:
		bs_readJointIndices(index, prim, data); break;
	    case cgltf_attribute_type_weights:
		bs_readWeights(index, prim, data); break;
    	}
    }

    if((load_settings & BS_INDICES) == BS_INDICES)
	bs_readIndices(mesh, model, c_mesh, prim_index);

    // TODO: Fixa att alla shared positioner ska ha samma unique_index
    int num_tris = prim->index_count;
    for(int i = 0; i < num_tris; i++) {
	int first_idx = 3 * i;

	for(int j = 0; j < 3; j++) {
	    int idx = prim->indices[first_idx + j];
	}
    }
}

void bs_loadJoints(cgltf_data *data, bs_Mesh *mesh, cgltf_mesh *c_mesh) {
    cgltf_skin *skin = c_mesh->node->skin;
    if(skin == NULL)
	    return;

    mesh->joints = malloc(skin->joints_count * sizeof(bs_Joint));
    mesh->joint_count = skin->joints_count;

    for(int i = 0; i < skin->joints_count; i++) {
	cgltf_node *c_joint = skin->joints[i];
	bs_Joint *joint = &mesh->joints[i];

	// Set the local matrix
	bs_mat4 local = GLM_MAT4_IDENTITY_INIT;
	glm_translate(local, c_joint->translation);
	glm_quat_rotate(local, (versor){ c_joint->rotation[0], c_joint->rotation[1], c_joint->rotation[2], c_joint->rotation[3] }, local);
	glm_scale(local, c_joint->scale);
	glm_mat4_inv(local, joint->local_inv);

	// Set the inverse bind matrix and regular bind matrix
	cgltf_accessor_read_float(skin->inverse_bind_matrices, i, (float*)joint->bind_matrix_inv, 16);
	glm_mat4_inv(joint->bind_matrix_inv, joint->bind_matrix);

	memcpy(mesh->joints[i].mat, GLM_MAT4_IDENTITY, sizeof(bs_mat4));

	c_joint->id = i;
    }

    for(int i = 0; i < skin->joints_count; i++) {
	int parent_id = skin->joints[i]->parent->id;

	// If parent id is the armature
	if(parent_id == -1) {
	    mesh->joints[i].parent = &identity_joint;
	    continue;
	}

	mesh->joints[i].parent = &mesh->joints[parent_id];
    }
}

void bs_loadMesh(cgltf_data *data, bs_Model *model, int mesh_index) {
    cgltf_mesh *c_mesh = &data->meshes[mesh_index];
    cgltf_node *node = &data->nodes[mesh_index];

    model->meshes[mesh_index].joint_count = 0;
    model->meshes[mesh_index].name = malloc(strlen(c_mesh->name));
    strcpy(model->meshes[mesh_index].name, c_mesh->name);

    memcpy(&model->meshes[mesh_index].pos, node->translation, sizeof(bs_vec3));
    memcpy(&model->meshes[mesh_index].rot, node->rotation, sizeof(bs_vec4));
    memcpy(&model->meshes[mesh_index].sca, node->scale, sizeof(bs_vec4));

    bs_mat4 local = GLM_MAT4_IDENTITY_INIT;
    glm_translate(local, node->translation);
    glm_quat_rotate(local, (versor){ node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3] }, local);
    glm_scale(local, node->scale);

    memcpy(&model->meshes[mesh_index].mat, &local, sizeof(bs_mat4));

    model->meshes[mesh_index].prims = malloc(c_mesh->primitives_count * sizeof(bs_Prim));
    model->meshes[mesh_index].prim_count = c_mesh->primitives_count;

    bs_loadJoints(data, &model->meshes[mesh_index], c_mesh);

    for(int i = 0; i < c_mesh->primitives_count; i++) {
	bs_loadPrim(data, &model->meshes[mesh_index], model, mesh_index, i);
    }
}

void bs_loadModelTexturesToAtlas() {
}

void bs_loadModelTextures(cgltf_data* data, bs_Model *model) {
	if(data->textures_count == 0)
		return;

	int64_t ids[data->textures_count];

	for(int i = 0; i < data->textures_count; i++) {
		// Getting the pointers to all images in the form of a 64 bit int
		ids[i] = (int64_t)data->textures[i].image;
	}

 	model->textures = malloc(data->textures_count * sizeof(bs_Tex2D));
	for(int i = 0; i < data->textures_count; i++) {
		char texture_path[256] = "resources/models/";
		strcat(texture_path, data->images[i].name);
		strcat(texture_path, ".png");

		//bs_loadTex2D(model->textures+i, texture_path);
		//bs_textureSettings(BS_LINEAR, BS_LINEAR);
		//bs_pushTexture(BS_CHANNEL_RGBA, BS_CHANNEL_RGBA, BS_UBYTE);
	}

 	curr_tex_ptr = ids[0];


	// for(int i = 0; i < data->textures_count; i++) {
	// 	ids[i] -= curr_tex_ptr;
	// 	// Divide by sizeof image so that stride between indices in ids is at most 1
	// 	ids[i] /= sizeof(cgltf_image);
	// 	model->textures[i] = images[ids[i]];
	// }
}

void bs_loadAnim(cgltf_data* data, int index, bs_Model *model) {
    cgltf_animation *c_anim = &data->animations[index];
    bs_Anim *anim = &anims[index];

    int joint_count = c_anim->samplers_count / 3;
    int frame_count = cgltf_accessor_unpack_floats(c_anim->samplers[0].input, NULL, 0);

    bs_mat4 *joints = malloc(joint_count * frame_count * sizeof(bs_mat4));
    anim->joint_count = joint_count;
    anim->frame_count = frame_count;
    anim->frame_offset_shader = anim_offset;
    anim_offset += frame_count * joint_count * sizeof(bs_mat4);

    int i, i3 = 0; 
    for(i = 0; i < joint_count; i++, i3+=3) {
	cgltf_animation_sampler *c_sampler = &c_anim->samplers[i3];
	cgltf_animation_channel *c_channel = &c_anim->channels[i3];

	// Input accessor contains timings for every frame in current joint
	// Output contains translation, rotation and scale for every frame in current joint
	cgltf_accessor *input = c_sampler->input;
	cgltf_accessor *translation_output = c_sampler->output + 0;
	cgltf_accessor *rotation_output    = c_sampler->output + 1;
	cgltf_accessor *scale_output       = c_sampler->output + 2; 

	for(int j = 0; j < frame_count; j++) {
	    int index = i + (j * joint_count);
	    bs_mat4 *joint = &joints[index];

	    bs_mat4 joint_mat = GLM_MAT4_IDENTITY_INIT;
	    vec3   tra;
	    versor rot;
	    vec3   sca;

	    cgltf_accessor_read_float(translation_output, j, (float*)&tra, 3);
	    cgltf_accessor_read_float(rotation_output   , j, (float*)&rot, 4);
	    cgltf_accessor_read_float(scale_output      , j, (float*)&sca, 3);

	    glm_translate(joint_mat, tra);
	    glm_quat_rotate(joint_mat, rot, joint_mat);
	    glm_scale(joint_mat, sca);

	    memcpy(joint, joint_mat, sizeof(bs_mat4));
	}
    }
    
    int ssbo_size = frame_count * joint_count * sizeof(bs_mat4);
    anim->matrices = malloc(ssbo_size);

    bs_Mesh *mesh = model->meshes;
    for(int i = 0; i < frame_count; i++) {
	for(int j = 0; j < joint_count; j++) {
	    bs_Joint *change_joint = &mesh->joints[j];
	    bs_Joint *parent = mesh->joints[j].parent;
	    int index = j + i * joint_count;

	    glm_mat4_mul(change_joint->bind_matrix, change_joint->local_inv, change_joint->mat);
	    glm_mat4_mul(change_joint->mat, joints[index], change_joint->mat);
	    glm_mat4_mul(change_joint->mat, change_joint->bind_matrix_inv, change_joint->mat);
	    glm_mat4_mul(parent->mat, change_joint->mat, change_joint->mat);

	    memcpy(anim->matrices + index, change_joint->mat, sizeof(bs_mat4));
	}
    }

    free(joints);
}

void bs_loadAnims(cgltf_data* data, bs_Model *model) {
    anim_count += data->animations_count;
    if(data->animations_count == 0)
	return;

    if(anim_count > allocated_anims) {
	allocated_anims = anim_count + 4;
	anims = realloc(anims, allocated_anims * sizeof(bs_Anim));
    }

    for(int i = 0; i < data->animations_count; i++)
	bs_loadAnim(data, i, model);
}

void bs_loadModel(char *model_path, bs_Model *model, int settings) {
    cgltf_options options = {0};
    cgltf_data* data = NULL;
    load_settings = settings;

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

    model->name = malloc(path_len);
    strcpy(model->name, model_path);

    // bs_loadModelTextures(data, model);
    for(int i = 0; i < mesh_count; i++) {
	bs_loadMesh(data, model, i);
    }
    bs_loadAnims(data, model);
}

void bs_animate(bs_Anim *anim, int frame) {
    frame %= anim->frame_count;
    frame *= 29;
    frame += anim->frame_offset_shader;

    bs_selectSSBO(anim_ssbo);
    bs_pushSSBO(&frame, 0, 4);
}

void bs_pushAnims() {
    int ssbo_size = anim_offset;
    anim_ssbo = bs_SSBO(NULL, ssbo_size + 16, 3);
    for(int i = 0; i < anim_count; i++) {
	bs_Anim *anim = anims + i;
	bs_pushSSBO(anim->matrices, 16 + anim->frame_offset_shader, ssbo_size);
	free(anim->matrices);
    }
}

bs_Anim *bs_getAnims() {
    return anims;
}
