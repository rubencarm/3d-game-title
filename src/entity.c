#include "../include/entity.h"

#include "simple_logger.h"

typedef struct
{
    Entity *entityList;
    Uint32 maxEnts;
}EntityManager;

static EntityManager entity_manager = {0};



void entity_close(){
    int i;
    if(!entity_manager.entityList)return;
    for(i = 0; i < entity_manager.maxEnts; i++){
        if(entity_manager.entityList[i]._inuse) entity_free(&entity_manager.entityList[i]);
    }
    free(entity_manager.entityList);
    memset(&entity_manager,0,sizeof(EntityManager));
    slog("entity system closed");
}


void entity_init(Uint32 maxEnts){
    if(!maxEnts)
    {
        slog("failed to initialize entity system: maxEnts is zero");
        return;
    }
    entity_manager.entityList = gfc_allocate_array(sizeof(Entity), maxEnts);
    if(!entity_manager.entityList)
    {
        slog("failed to allocate entity list for %i entities", maxEnts);
        return;
    }
    entity_manager.maxEnts = maxEnts;
    slog("entity system initialized");
    atexit(entity_close);

}

Entity *entity_new()
{
    int i;
    for(i = 0; i < entity_manager.maxEnts; i++)
    {
        if(entity_manager.entityList[i]._inuse) continue;
        entity_manager.entityList[i]._inuse = 1;
        // boilerplate goes here

        entity_manager.entityList[i].scale = gfc_vector3d(1,1,1);
        entity_manager.entityList[i].colorMod = GFC_COLOR_WHITE;
        return &entity_manager.entityList[i];
    }
}

void entity_system_draw()
{
    int i;
    for(i = 0; i < entity_manager.maxEnts; i++){
        if(!entity_manager.entityList[i]._inuse) continue;
    }
}

void entity_draw(Entity* ent){

    if((!ent) || (!ent->_inuse)) return;
    if(ent->draw){//do we have a draw function
        if(ent->draw(ent))return; //does this draw function handle queueing render?
    }
    gfc_matrix4_from_vectors(
        ent->mat,
        ent->position,
        ent->rotation,
        ent->scale);
    model_queue_render(ent->model, ent->mat, ent->colorMod);

}

void entity_free(Entity* ent){
    if(!ent) return;
    if(!ent->_inuse)return;

    model_free(ent->model);
    memset(ent,0,sizeof(Entity));

}

void entity_update(Entity *self)
{

    if((!self) || (!self->_inuse)) return;
    if(self->update)
    {
        if(self->update(self)) return;
    }
    gfc_vector3d_add(self->position, self->position, self->velocity);
}

void entity_system_update()
{

}
