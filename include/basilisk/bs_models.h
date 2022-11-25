#ifndef BS_MODELS_H
#define BS_MODELS_H

#include <bs_types.h>

void bs_model(bs_Model *model, const char *model_path, int settings);
void bs_animate(bs_Anim *anim, int frame);
void bs_pushAnims();
bs_Anim *bs_getAnims();
bs_Anim *bs_getAnimFromName(char *name);
void bs_freeModel(bs_Model *model);

/* --- SETTINGS --- */
/* Loads the indices (Default) */
#define BS_INDICES 1

/* Generates another array containing unique indices, based on vertex positions */
#define BS_INDICES_UNIQUE 2

/* Generates another array containing default indices and adjacent indices, 
 * this also generates default indices, but frees them unless BS_INDICES is set.
 * this also generates unique indices, but frees them unless BS_INDICES_UNIQUE is set. */
#define BS_INDICES_ADJACENT 4

#define BS_MDEFAULT BS_INDICES

#endif /* BS_MODELS_H */
