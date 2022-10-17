#ifndef BS_MODELS_H
#define BS_MODELS_H

#include <bs_types.h>

void bs_loadModel(char *model_path, char *texture_folder_path, bs_Model *model);
void bs_animate(bs_Anim *anim, int frame);
void bs_pushAnims();
bs_Anim *bs_getAnims();

#endif /* BS_MODELS_H */
