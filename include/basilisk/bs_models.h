#ifndef BS_MODELS_H
#define BS_MODELS_H

#include <bs_types.h>

int bs_model(bs_Model *model, const char *model_path, const char *texture_path, bool find_unique_indices);
void bs_animation(bs_Anim *anim, bs_Skin *skin);
void bs_animate(bs_Refs refs, bs_U32 frame, bs_Anim *anim);
void bs_pushAnims();
int bs_jointOffsetFromName(bs_Skin *skin, const char *name);
bs_mat4 bs_matrixFrameFromJoint(bs_Anim *anim, bs_U32 frame, int joint_offset);
bs_Anim *bs_anims();
bs_Anim *bs_modelAnims(bs_Model *model);
bs_Anim *bs_modelAnimFromName(bs_Model *model, const char *name);
bs_Anim *bs_animFromName(const char *name);
bs_Skin *bs_skinFromName(const char *name, bs_Model *model);
bs_Mesh *bs_meshFromName(const char *name, bs_Model *model);
int bs_meshIdxFromName(const char *name, bs_Model *model);
int bs_numAnims();
void bs_freeModel(bs_Model *model);

#endif /* BS_MODELS_H */
