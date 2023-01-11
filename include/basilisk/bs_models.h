#ifndef BS_MODELS_H
#define BS_MODELS_H

#include <bs_types.h>

int bs_model(bs_Model *model, const char *model_path, int settings);
void bs_animation(int bind_point, bs_Anim *anim, bs_Mesh *mesh);
void bs_animate(bs_Anim *anim, int bind_point, int frame);
void bs_pushAnims();
bs_Anim *bs_anims();
bs_Anim *bs_animFromName(const char *name);
int bs_numAnims();
void bs_freeModel(bs_Model *model);

/* --- SETTINGS --- */
/* Loads the indices (Default) */
#define BS_INDICES 1

/* Generates another array containing unique indices, based on vertex positions */
#define BS_INDICES_UNIQUE 2

/* Generates another array containing default indices and adjacent indices, 
 * this also generates unique indices, but frees them unless BS_INDICES_UNIQUE is set. */
#define BS_INDICES_ADJACENT 4

#define BS_MDEFAULT BS_INDICES

#endif /* BS_MODELS_H */
