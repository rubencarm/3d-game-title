#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "model.h"

typedef struct entity_s Entity;

struct entity_s
{
    Uint8           _inuse;
    Model           *model;
    GFC_Vector3D    position;
    GFC_Vector3D    rotation;
    GFC_Vector3D    scale;
    GFC_Matrix4     mat;

    GFC_Vector3D    velocity;
    GFC_Color       colorMod;

    int     (*think)(Entity* self);
    int     (*update)(Entity* self);
    int     (*draw)(Entity* self);

};
/**
 * @brief initialize the entity manager and preallocate them
 * @param maxEnts how many entities can exist at a single moment
 */
void entity_init(Uint32 MaxEnts);

/**
 * @brief get a blank entity from the system
 * @return NULL on error or OOM, a valid pointer otherwise
 */
Entity *entity_new();

/**
 * @brief call to clean up an entity after it is no longer needed
 */
void entity_free(Entity *ent);

/**
 * @brief run updates on all active entities
 */
void entity_system_update();

/**
 * @brief finkabout it
 */
void entity_system_think();

/**
 * @brief queue renders on all active entities
 */
void entity_draw_think();

#endif
