#ifndef SCRIPTUP_SKETCHUP_H
#define SCRIPTUP_SKETCHUP_H

#include <SketchUpAPI/sketchup.h>

SUModelRef Sketchup_get_active_model();
void Sketchup_set_active_model(SUModelRef model);

#endif // SCRIPTUP_SKETCHUP_H