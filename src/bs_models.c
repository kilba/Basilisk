#include <stdio.h>
#include <time.h>
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
#include <bs_debug.h>

bs_Joint identity_joint = { BS_MAT4_IDENTITY_INIT };

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

/* --- VERTEX PARSING --- */
void bs_modelAttribData(int accessor_idx, int offset, bs_Prim *prim, cgltf_data *data) {
    cgltf_accessor *accessor = data->accessors + accessor_idx;
    int num_floats = cgltf_accessor_unpack_floats(accessor, NULL, 0);
    int num_comps = cgltf_num_components(accessor->type);

    for(int i = 0; i < num_floats / num_comps; i++, offset += prim->vertex_size) {
	cgltf_accessor_read_float(accessor, i, (float *)prim->vertices + offset, num_comps);
    }
}

void bs_modelAttribDataI(int accessor_idx, int offset, bs_Prim *prim, cgltf_data *data) {
    cgltf_accessor *accessor = data->accessors + accessor_idx;
    int num_floats = cgltf_accessor_unpack_floats(accessor, NULL, 0);
    int num_comps = cgltf_num_components(accessor->type);

    for(int i = 0; i < num_floats / num_comps; i++, offset += prim->vertex_size) {
	cgltf_accessor_read_uint(accessor, i, (unsigned int *)prim->vertices + offset, num_comps);
    }
}

void bs_loadMaterial(bs_Model *model, cgltf_data *c_data, bs_Material *material, cgltf_material *c_material) {
    // Default values
    material->data.color = bs_v4(0.5, 0.5, 0.0, 1.0);
    material->texture_handle = bs_defTexture()->handle;

    if(c_material == NULL)
	return;

    // Actual values
    cgltf_pbr_metallic_roughness *metallic = &c_material->pbr_metallic_roughness;
    cgltf_float *color = metallic->base_color_factor;

    material->data.color = bs_v4(color[0], color[1], color[2], color[3]);

    // Texture handling
    cgltf_texture *tex = metallic->base_color_texture.texture;
    
    material->shader_material = bs_shaderMaterialInit(material);

    if(tex == NULL)
	return;

    // Convert texture pointer to array index
    uint64_t id = (uint64_t)tex->image;
    id -= (uint64_t)c_data->textures[0].image;
    id /= sizeof(cgltf_image);

    if(id >= model->texture_count)
	return;

    material->texture_handle = model->textures[id].handle;
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
    mesh->index_count += prim->index_count;
    model->vertex_count += prim->vertex_count;
    model->index_count += num_indices;
}

void bs_loadPrim(cgltf_data *data, bs_Mesh *mesh, bs_Model *model, int mesh_index, int prim_index) {
    cgltf_mesh *c_mesh = data->meshes + mesh_index;
    cgltf_primitive *c_prim = c_mesh->primitives + prim_index;
    bs_Prim *prim = &mesh->prims[prim_index];

    int attrib_count = c_mesh->primitives[prim_index].attributes_count;
    int num_floats = cgltf_accessor_unpack_floats(&data->accessors[c_mesh->primitives[prim_index].attributes[0].index], NULL, 0);

    int vertex_size = 0;

    prim->parent = mesh;
    prim->offset_nor = 0;
    prim->offset_tex = 0;
    prim->offset_bid = 0;
    prim->offset_wei = 0;
    prim->offset_tex = 0;

    // Calculate vertex size
    for(int i = 0; i < attrib_count; i++) {
    	int index = c_mesh->primitives[prim_index].attributes[i].index;
    	int type = c_mesh->primitives[prim_index].attributes[i].type;

	// Position has no offset since it is always at offset 0 
    	switch(type) {
	    case cgltf_attribute_type_position : vertex_size += 3; break;
	    case cgltf_attribute_type_normal   : prim->offset_nor = vertex_size; vertex_size += 3; break;
	    case cgltf_attribute_type_texcoord : prim->offset_tex = vertex_size; vertex_size += 2; break;
	    case cgltf_attribute_type_joints   : prim->offset_bid = vertex_size; vertex_size += 4; break;
	    case cgltf_attribute_type_weights  : prim->offset_wei = vertex_size; vertex_size += 4; break;
    	}
    }

    prim->vertex_size = vertex_size;
    prim->vertex_count = num_floats / 3;
    prim->vertices = malloc(prim->vertex_count * vertex_size * sizeof(float));

    // Read vertices
    for(int i = 0; i < attrib_count; i++) {
    	int index = c_mesh->primitives[prim_index].attributes[i].index;
    	int type = c_mesh->primitives[prim_index].attributes[i].type;

    	switch(type) {
	    case cgltf_attribute_type_position : bs_modelAttribData (index, 0, prim, data); break;
	    case cgltf_attribute_type_normal   : bs_modelAttribData (index, prim->offset_nor, prim, data); break;
	    case cgltf_attribute_type_texcoord : bs_modelAttribData (index, prim->offset_tex, prim, data); break;
	    case cgltf_attribute_type_joints   : bs_modelAttribDataI(index, prim->offset_bid, prim, data); break;
	    case cgltf_attribute_type_weights  : bs_modelAttribData (index, prim->offset_wei, prim, data); break;
    	}
    }

    bs_readIndices(mesh, model, c_mesh, prim_index);

    // Get material index for primitive (model->materials[prim->material_idx])
    if(c_prim->material == NULL) {
	prim->material_idx = 0;
	return;
    }

    // Convert material pointer to array index
    uint64_t mat_id = (uint64_t)c_prim->material;
    mat_id -= (uint64_t)data->materials;
    mat_id /= sizeof(cgltf_material);
    prim->material_idx = mat_id + 1;
}

void bs_loadMesh(cgltf_data *data, bs_Model *model, int mesh_index) {
    cgltf_mesh *c_mesh = &data->meshes[mesh_index];
    cgltf_node *node = &data->nodes[mesh_index];

    bs_Mesh *mesh = model->meshes + mesh_index;

    int c_mesh_name_len = strlen(c_mesh->node->name);
    mesh->parent = model;
    mesh->id = mesh_index;
    mesh->index_count = 0;
    mesh->vertex_count = 0;
    mesh->name = malloc(c_mesh_name_len+1);
    int n = sprintf(mesh->name, "%s", c_mesh->node->name);

    memcpy(&mesh->pos, node->translation, sizeof(bs_vec3));
    memcpy(&mesh->rot, node->rotation, sizeof(bs_vec4));
    memcpy(&mesh->sca, node->scale, sizeof(bs_vec4));
   
    bs_mat4 local = BS_MAT4_IDENTITY_INIT;
    glm_translate(local.a, node->translation);
    glm_quat_rotate(local.a, node->rotation, local.a);
    glm_scale(local.a, node->scale);
 
    memcpy(mesh->mat.a, &local, sizeof(bs_mat4));

    mesh->prims = malloc(c_mesh->primitives_count * sizeof(bs_Prim));
    mesh->prim_count = c_mesh->primitives_count;

    for(int i = 0; i < c_mesh->primitives_count; i++)
	bs_loadPrim(data, mesh, model, mesh_index, i);

}

void bs_checkMeshAnimResize(bs_Anim *anim) {
    const int realloc_by = 8;
    if((anim->num_mesh_anims % realloc_by) != 0)
	return;

    size_t size = (realloc_by + anim->num_mesh_anims) * sizeof(bs_MeshAnim);
    anim->mesh_anims = realloc(anim->mesh_anims, size);
}

void bs_jointlessAnimation(bs_Anim *anim, bs_Mesh *mesh) {
    bs_checkMeshAnimResize(anim);
    bs_MeshAnim *mesh_anim = anim->mesh_anims + anim->num_mesh_anims;

    mesh_anim->joints = malloc(anim->frame_count * sizeof(bs_mat4));
    mesh_anim->num_joints = anim->joint_count;
    mesh_anim->num_frames = anim->frame_count;
    mesh_anim->shader_offset = anim_offset;
    anim_offset += anim->frame_count;

    // For each FRAME in animation
    for(int i = 0; i < anim->frame_count; i++)
	memcpy(mesh_anim->joints[i].a, anim->matrices[i].a, sizeof(bs_mat4));

    anim->num_mesh_anims++;
}

void bs_animation(bs_Anim *anim, bs_Skin *skin) {
    if(anim == NULL) {
	printf("Animation \"%s\" is NULL\n", anim->name);
	exit(1);
    }

    if(skin == NULL) {
	printf("Skin \"%s\" is NULL\n", skin->name);
	exit(1);
    }

    if(anim->joint_count != skin->joint_count) {
	//bs_jointlessAnimation(anim, mesh)
	printf("Joint mismatch:\n    %s: %d\n    %s: %d\n", anim->name, anim->joint_count, skin->name, skin->joint_count);
	exit(1);
    }

    bs_checkMeshAnimResize(anim);
    bs_MeshAnim *mesh_anim = anim->mesh_anims + anim->num_mesh_anims;

    mesh_anim->joints = malloc(anim->frame_count * anim->joint_count * sizeof(bs_mat4));
    mesh_anim->num_joints = anim->joint_count;
    mesh_anim->num_frames = anim->frame_count;
    mesh_anim->shader_offset = anim_offset;
    anim_offset += anim->frame_count * anim->joint_count;

    // For each FRAME in animation
    for(int i = 0; i < anim->frame_count; i++) {
	// For each JOINT in mesh
	for(int j = 0; j < anim->joint_count; j++) {
	    bs_Joint *change_joint = &skin->joints[j];
	    bs_Joint *parent = skin->joints[j].parent;
	    int idx = j + i * anim->joint_count;

	    // RESULT_JOINT  = (BIND MATRIX) * (LOCAL INVERSE)
	    // RESULT_JOINT *= (ANIMATION JOINT OF CURRENT FRAME)
	    // RESULT_JOINT *= (INVERSE BIND MATRIX)
	    // RESULT_JOINT  = (JOINT PARENT) * (RESULT_JOINT)

	    glm_mat4_mul(change_joint->bind_matrix.a, change_joint->local_inv.a, change_joint->mat.a);
	    glm_mat4_mul(change_joint->mat.a, anim->matrices[idx].a, change_joint->mat.a);
	    glm_mat4_mul(change_joint->mat.a, change_joint->bind_matrix_inv.a, change_joint->mat.a);
	    glm_mat4_mul(parent->mat.a, change_joint->mat.a, change_joint->mat.a);
	    
	    memcpy(mesh_anim->joints + idx, change_joint->mat.a, sizeof(bs_mat4));
	}
    }

    anim->num_mesh_anims++;
}

int bs_jointOffsetFromName(bs_Skin *skin, const char *name) {
    for(int i = 0; i < skin->joint_count; i++) {
	if(strcmp(skin->joints[i].name, name) == 0)
	    return i;
    }

    return -1;
}

bs_mat4 bs_matrixFrameFromJoint(bs_Anim *anim, bs_U32 frame, int joint_offset) {
    if(joint_offset < 0)
	return BS_MAT4_IDENTITY;
    if(joint_offset >= anim->joint_count)
	return BS_MAT4_IDENTITY;

    return anim->matrices[joint_offset + anim->joint_count * frame];
}

void bs_loadAnim(cgltf_data* data, int index, int old_anim_offset, bs_Model *model) {
    cgltf_animation *c_anim = &data->animations[index];
    bs_Anim *anim = &anims[index + old_anim_offset];

    int joint_count = c_anim->samplers_count / 3;
    int frame_count = cgltf_accessor_unpack_floats(c_anim->samplers[0].input, NULL, 0);

    int name_len = strlen(c_anim->name);

    bs_mat4 *joints = malloc(joint_count * frame_count * sizeof(bs_mat4));
    anim->name = malloc(name_len + 1);
    strcpy(anim->name, c_anim->name);

    anim->model = model;
    anim->mesh_anims = NULL;
    anim->num_mesh_anims = 0;
    
    anim->joint_count = joint_count;
    anim->frame_count = frame_count;

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
	    int idx = i + (j * joint_count);

	    bs_vec3 tra;
	    bs_quat rot;
	    bs_vec3 sca;

	    cgltf_accessor_read_float(translation_output, j, (float*)&tra, 3);
	    cgltf_accessor_read_float(rotation_output   , j, (float*)&rot, 4);
	    cgltf_accessor_read_float(scale_output      , j, (float*)&sca, 3);

	    joints[idx] = bs_transform(tra, rot, sca);
	}
    }

    anim->matrices = joints;
}

void bs_loadAnims(cgltf_data* data, bs_Model *model) {
    int old_anim_count = anim_count;
    anim_count += data->animations_count;

    if(data->animations_count == 0)
	return;
    
    model->anim_offset = old_anim_count;
    model->anim_count = data->animations_count;

    if(anim_count > allocated_anims) {
	allocated_anims = anim_count + 8;
	anims = realloc(anims, allocated_anims * sizeof(bs_Anim));
    }

    for(int i = 0; i < data->animations_count; i++)
	bs_loadAnim(data, i, old_anim_count, model);
}

void bs_loadTexturePath(int idx, bs_Model *model, cgltf_texture *c_texture, const char *texture_path) {
    // TODO: Check if already ending with .png (also .jpg support)
    char *path = malloc(strlen(texture_path) + strlen(c_texture->image->name) + sizeof(".png"));
    sprintf(path, "%s/%s.png", texture_path, c_texture->image->name);
    
    bs_texturePNG(model->textures + idx, path);
    printf("%s\n", path);

    model->texture_names[idx] = path;
}

void bs_loadSkin(cgltf_skin *c_skin, bs_Skin *skin) {
    int strlen_skin_name = strlen(c_skin->name);

    skin->name = malloc(strlen_skin_name + 1);
    strncpy(skin->name, c_skin->name, strlen_skin_name);
    skin->name[strlen_skin_name] = '\0';

    skin->joints = malloc(c_skin->joints_count * sizeof(bs_Joint));
    skin->joint_count = c_skin->joints_count;

    for(int i = 0; i < skin->joint_count; i++) {
	cgltf_node *c_joint = c_skin->joints[i];
	bs_Joint *joint = skin->joints + i;

	// Set the local matrix
	bs_mat4 local = BS_MAT4_IDENTITY_INIT;
	glm_translate(local.a, c_joint->translation);
	glm_quat_rotate(local.a, c_joint->rotation, local.a);
	glm_scale(local.a, c_joint->scale);
	glm_mat4_inv(local.a, joint->local_inv.a);

	// Set the inverse bind matrix and regular bind matrix
	cgltf_accessor_read_float(c_skin->inverse_bind_matrices, i, (float*)joint->bind_matrix_inv.a, 16);
	glm_mat4_inv(joint->bind_matrix_inv.a, joint->bind_matrix.a);

	// Set name
	int joint_name_len = strlen(c_joint->name);
	joint->name = malloc(joint_name_len + 1);
	strncpy(joint->name, c_joint->name, joint_name_len);
	joint->name[joint_name_len] = '\0';

	joint->mat = BS_MAT4_IDENTITY;

	c_joint->id = i;
    }

    for(int i = 0; i < skin->joint_count; i++) {
	int parent_id = c_skin->joints[i]->parent->id;

	// If parent id is the armature
	if(parent_id == -1) {
	    skin->joints[i].parent = &identity_joint;
	    continue;
	}

	skin->joints[i].parent = skin->joints + parent_id;
    }
}

void bs_cgltfError(int err) {
    switch(err) {
	case cgltf_result_success: return;
	case cgltf_result_unknown_format: printf("GLTF ERROR: \"UNKNOWN FORMAT\"\n"); break;
	case cgltf_result_invalid_json: printf("GLTF ERROR: \"INVALID JSON\"\n"); break;
	case cgltf_result_invalid_gltf: printf("GLTF ERROR: \"INVALID GLTF\"\n"); break;
	case cgltf_result_file_not_found: printf("GLTF ERROR: \"FILE NOT FOUND\"\n"); break;
	default: printf("GLTF ERROR (code %d)\n", err); break;
    }
}

int bs_model(bs_Model *model, const char *model_path, const char *texture_path) {
    cgltf_options options = {0};
    cgltf_data* data = NULL;

    int err;
    err = cgltf_parse_file(&options, model_path, &data);
    if(err != cgltf_result_success) return err;
    err = cgltf_load_buffers(&options, data, model_path);
    if(err != cgltf_result_success) return err;
    
    int path_len = strlen(model_path);
    int mesh_count = data->meshes_count;

    model->texture_names = NULL;
    model->meshes = malloc(mesh_count * sizeof(bs_Mesh));
    model->mesh_count = mesh_count;
    model->prim_count = 0;
    model->anim_count = 0;
    model->anim_offset = 0;
    model->vertex_count = 0;
    model->index_count = 0;
 
    model->name = malloc(path_len + 1);
    strcpy(model->name, model_path);

    model->textures      = malloc(data->textures_count * sizeof(bs_Texture));
    model->texture_names = malloc(data->textures_count * sizeof(char *));
    model->texture_count = data->textures_count;
    model->material_count = data->materials_count + 1;

    model->materials = malloc(model->material_count * sizeof(bs_Material));

    // Load textures
    for(int i = 0; i < data->textures_count; i++) {
	bs_loadTexturePath(i, model, data->textures + i, texture_path);
    }

    // Load materials
    model->materials[0] = (bs_Material){ (bs_ShaderMaterial){ bs_v4(0.5, 0.5, 0.0, 1.0) }, 0, bs_defTexture()->handle, 0.0 };
    model->materials[0].shader_material = bs_shaderMaterialInit(model->materials);

    for(int i = 1; i < model->material_count; i++) {
	bs_loadMaterial(model, data, model->materials + i, data->materials + i - 1);
    }

    // Load skins
    model->skins = malloc(data->skins_count * sizeof(bs_Skin));
    model->skin_count = data->skins_count;
    for(int i = 0; i < data->skins_count; i++) {
	bs_loadSkin(data->skins + i, model->skins + i);
    }

    // Load meshes (also loads primitives)
    for(int i = 0; i < mesh_count; i++) {
	bs_loadMesh(data, model, i);
	model->prim_count += model->meshes[i].prim_count;
    }

    bs_loadAnims(data, model);

    bs_Idxs idxs = {
	bs_shaderModelInit(BS_MAT4_IDENTITY),
	0,
	0,
	bs_defTexture()->handle
    };
    model->refs = bs_shaderModelReferences(model, idxs);

    cgltf_free(data);
    return 0;
}

void bs_animate(bs_Refs refs, bs_U32 frame, bs_Anim *anim) {
    if(anim == NULL) {
	printf("Anim is NULL\n");
	return;
    }

    struct {
	int frame;
	int num_frames;
    } buf; 

    frame %= anim->frame_count;
    frame *= anim->joint_count;
    frame += anim->mesh_anims->shader_offset;
    buf.frame = frame;
    buf.num_frames = anim->frame_count;

    bs_updateShaderFrame(frame, refs);
}

void bs_pushAnims() {
    int ssbo_size = anim_offset * sizeof(bs_mat4);
    anim_ssbo = bs_SSBO(NULL, ssbo_size + sizeof(int) * 4, 1);
 
    for(int i = 0; i < anim_count; i++) {
	bs_Anim *anim = anims + i;
	for(int j = 0; j < anim->num_mesh_anims; j++) {
	    bs_MeshAnim *mesh_anim = anim->mesh_anims + j;

	    int size   = mesh_anim->num_joints * anim->frame_count * sizeof(bs_mat4);
	    int offset = (sizeof(int) * 4) + mesh_anim->shader_offset * sizeof(bs_mat4);
	    
	    bs_pushSSBO(mesh_anim->joints, offset, size);
	    free(mesh_anim->joints);
	}

	//free(anim->mesh_anims);
	free(anim->matrices);
    }
}

bs_Mesh *bs_meshFromName(const char *name, bs_Model *model) {
    for(int i = 0; i < model->mesh_count; i++) {
	if(strcmp(name, model->meshes[i].name) == 0)
	    return model->meshes + i;
    }

    return NULL;
}

int bs_meshIdxFromName(const char *name, bs_Model *model) {
    bs_Mesh *mesh = bs_meshFromName(name, model);
    if(mesh == NULL)
	return -1;

    bs_U64 ptr0 = (bs_U64)model->meshes;
    bs_U64 ptr1 = (bs_U64)mesh;

    return (ptr1 - ptr0) / sizeof(bs_Mesh);
}

bs_Skin *bs_skinFromName(const char *name, bs_Model *model) {
    for(int i = 0; i < model->skin_count; i++) {
	bs_Skin *skin = model->skins + i;
	if(strcmp(name, skin->name) == 0)
	    return skin;
    }
    return NULL;
}

bs_Anim *bs_anims() {
    return anims;
}

bs_Anim *bs_modelAnims(bs_Model *model) {
    if(model->anim_count == 0)
	return NULL;

    return anims + model->anim_offset;
}

bs_Anim *bs_modelAnimFromName(const char *name, bs_Model *model) {
    for(int i = 0; i < model->anim_count; i++) {
	bs_Anim *anim = anims + model->anim_offset + i;
	if(strcmp(name, anim->name) == 0)
	    return anim;
    }

    return NULL;
}

bs_Anim *bs_animFromName(const char *name) {
    for(int i = 0; i < anim_count; i++) {
	bs_Anim *anim = anims + i;
	if(strcmp(name, anim->name) == 0)
	    return anim;
    }

    return NULL;
}

int bs_numAnims() {
    return anim_count;
}

void bs_freeModel(bs_Model *model) {
    for(int m = 0; m < model->mesh_count; m++) {
	bs_Mesh *mesh = model->meshes + m;
	for(int p = 0; p < mesh->prim_count; p++) {
	    bs_Prim *prim = mesh->prims + p;
	    free(prim->indices);
	    free(prim->vertices);
	}

	free(mesh->prims);
    }

    for(int i = 0; i < model->skin_count; i++) {
	bs_Skin *skin = model->skins + i;

	for(int j = 0; j < skin->joint_count; j++)
	    free(skin->joints);
    }

    // TODO: Free texture names?
    free(model->skins);
    free(model->meshes);
}
