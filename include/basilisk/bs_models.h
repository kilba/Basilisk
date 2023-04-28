#ifndef BS_MODELS_H
#define BS_MODELS_H

#include <bs_types.h>

int bs_model(bs_Model *model, const char *model_path, const char *texture_path);
void bs_animation(bs_Anim *anim, bs_Skin *skin);
void bs_animate(bs_Anim *anim, int bind_point, int frame);
void bs_pushAnims();
bs_Anim *bs_anims();
bs_Anim *bs_modelAnims(bs_Model *model);
bs_Anim *bs_modelAnimFromName(const char *name, bs_Model *model);
bs_Anim *bs_animFromName(const char *name);
bs_Skin *bs_skinFromName(const char *name, bs_Model *model);
int bs_numAnims();
void bs_freeModel(bs_Model *model);

#endif /* BS_MODELS_H */
