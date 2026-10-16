#include "simple_logger.h"

#include "player.h"

Entity *player_spawn(GFC_Vector3D position)
{
    Entity *self;
    self = entity_new();
    if(!self) return NULL;
    self->model = model_load("models/dino/dino.model");
    self->position = position;
    return self;
}
