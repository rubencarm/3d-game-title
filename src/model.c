#include "gf3d_mesh.h"
#include "gf3d_pipeline.h"
#include "gf3d_texture.h"
#include "gfc_types.h"
#include "simple_logger.h"

#include "model.h"
#include "gf3d_vgraphics.h"

extern int _DEBUG;

typedef struct
{
	Model 		*modelList;
	Uint32 		modelCount;
	Pipeline	*pipe;
	VkDevice 	device;
	Texture		*defaultTexture;
}ModelManager;

static ModelManager model_manager = {0};

void model_manager_close(void) {
    int i;
    gf3d_texture_free(model_manager.defaultTexture);

    for (i = 0; i < model_manager.modelCount; i++) {
        model_free(&model_manager.modelList[i]);
    }
    memset(model_manager.modelList, model_manager.modelCount, '0');
    return;
}


void model_init_system(Uint32 model_max)
{
	if(model_manager.modelCount != 0){
		slog("cannot init mesh system, already initialized");
		return;
	}
	if(!model_max){
		slog("cannot initialize model system for zero models");

	}
	model_manager.modelList = gfc_allocate_array(sizeof(Model), model_max);
    if (!model_manager.modelList) {
        slog("failed to initialize modelList"); return;}
	model_manager.modelCount = model_max;
    model_manager.device = gf3d_vgraphics_get_default_logical_device();
    gf3d_mesh_init(model_max);
    slog("model system initialized");
    atexit(model_manager_close);
}

void model_close(void)
{
    int i, c;
    c = model_manager.modelCount;
    for(i = 0; i < c; i++)
    {
        model_free(&model_manager.modelList[i]);
    }
    free(model_manager.modelList);
    gf3d_pipeline_free(model_manager.pipe);
    gf3d_texture_free(model_manager.defaultTexture);
    memset(&model_manager,0,sizeof(ModelManager));
}

void model_free(Model *model)
{
    if(!model) return;

    gf3d_texture_free (model->texture);
    gf3d_mesh_free(model->mesh);
    memset(model,0,sizeof(Model));
}
Model *model_new(void){
    int i;
    for(int i = 0; i < model_manager.modelCount; i++)
    {
        if(model_manager.modelList[i]._refCount) continue;
        return &model_manager.modelList[i];
    }
    return NULL;
}

Model *model_get_by_filename(const char *filename)
{
    int i;
    if(!filename) return NULL;
    for(i = 0; i < model_manager.modelCount; i++)
    {
        if(model_manager.modelList[i]._refCount == 0) continue;
        if(gfc_strlcmp(filename, model_manager.modelList[i].mesh->filename) == 0)
        {
            return &model_manager.modelList[i];
        }
        else
        {
            gfc_line_cpy(model_manager.modelList[i].mesh->filename, filename);
            model_manager.modelList[i]._refCount++;
            return &model_manager.modelList[i];

        }
    }
    return NULL;
}

Model *model_load_from_file(const char *filename) {
    int i;
    if (!filename){
        return NULL;
    }
    for (i = 0; i < model_manager.modelCount; i++) {

    }

    return NULL;


}

Model *model_load(const char *filename)
{
    const char *str = NULL;
    const char *str2 = NULL;
    Mesh *mesh = NULL;
    Texture *texture = NULL;
    SJson *json = NULL,*data = NULL;
    Model *model = NULL;
    if (!filename)
        return NULL;
    model_load_from_file(filename);
    if (!model)
        model = model_new();
    if (!model)
        return NULL;
    json = sj_load(filename);
    if (!json) {
        model_free(model);
        return NULL;
    }
    data = sj_object_get_value(json, "model");
    if (!data) {
        model_free(model);
        sj_free(json);
        return NULL;
    }
    str = sj_object_get_value_as_string(data, "obj");
    if (!str) {
        model_free(model);
        sj_free(json);
        return NULL;
    }
    mesh = gf3d_mesh_load_obj(str);
    if (!mesh) {
        model_free(model);
        sj_free(json);
        return NULL;
    }
    str2 = sj_object_get_value_as_string(data, "texture");
    if (!str2) {
        model->texture = model_manager.defaultTexture;
    } else {
        slog("here!!!");
        slog("%s", str2);
        texture = gf3d_texture_load(str2);
        if (!texture) {
            texture = model_manager.defaultTexture;
            slog("default texture");
        }
    }

    model->mesh = mesh;
    model->texture = texture;
    gfc_line_cpy(model->mesh->filename, filename);
    return model;
}





void model_queue_render(Model *model, GFC_Matrix4 modelMat,
                        GFC_Color colorMod) {
    MeshUBO ubo = gf3d_mesh_get_ubo(modelMat,colorMod);
    gf3d_mesh_queue_render(model->mesh, gf3d_mesh_get_pipeline(), &ubo, model->texture);

}
