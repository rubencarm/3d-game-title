#include "model.h"

typedef struct
{
	Model 		*modelList;
	Uint32 		modelCount;
	Pipeline	*pipe;
	VkDevice 	device;
	Texture		*defaultTexture;
}ModelManager

static ModelManager model_manager = {0};

void model_init_system(Uint32 model_max)
{
	if(model_manager.modelCount != 0){
		slog("cannot init mesh system, already initialized");
		return;
	}
	if(
}

ModelUBO model_get_ubo)
	GFC_Matrix4 modelMat,
	GFC_Color colorMod)
{
	ModelUBO ubo = {0};
	GFC_Matrix4 *view;
	gfc_matrix4_copy(ubo.model,modelMat);
	view = gf3d_vgraphics_get_view_matrix();
	if(view) gfc_matrix4_copy(ubo.view,*view);
	gf3d_vgraphics_get_projection_matrix(&ubo.proj);
	ubo.color = gfc_color_to_vector4f(colorMod);
	return ubo;
}
void model_close()
{
	int i,c;
	for(i = 0; i < model_manager.modelCount; i++)
	{
		model_free(&model_manager.modelList[i]);
	}
	free(model_manager.modelList);
	gf3d_pipeline_free(model_manager.pipe);
	gf3d_texture_free(model_manager.defaultTexture);
	memset(&model_manager,0,sizeof(ModelManager));
}

void model_delete(Model *model)
{
	if(!model) return;

	gf3d_texture_free (model->texture);
	gf3d_mesh_free(model->mesh);
	memset(model,0,sizeof(model));
}
Mesh *model_new(){
	int i;
	for(int i = 0; i < model_manager.modelCount; i++)
	{
		if((model_manager.modelList[i]._refCount == 0) && (strlen(model_manager.modelList[i].filename) == 0)

	}

}
Model *model_get_by_filename(const char *filename)
{
	int i;
	if(!filename) return NULL;
	for(i = 0; i < model_manager.modelCount; i++)
	{
		if(model_manager.modelList[i]._refCount == 0) continue;
		if(gfc_strlcmp(filename, model_manager.modelList[i].filename) == 0){
			return &model_manager.modelList[i];
		}
	}
	return NULL;
}

Model *model_load(const char *filename)
{
	const char *str = NULL;
	const char *str2 = NULL;
	Mesh *mesh;
	Texture *texture;
	SJson *json,*data;
	Model *model;
	if(!filename) return NULL;
	model = model_get_by_filename(filename);
	if(model){
		model->_refCount++;
		return model;
	}
	if(!model)
	{
		slog("failed to get empty space in memory for model %s", filename);
		return NULL;
	}
	json = sj_load(filename);
	if(!json)
	{
		slog("failed to load model %s", filename);
		return NULL;
	}
	data = sj_object_get_value(json,"model");
	if(!data)
	{
		slog("failed to get model info from file %s", filename);
		sj_free(json);
		return NULL;
	}

	str = sj_object_get_string(data,"obj");
	if(!str){
		slog("failed to find obj data in file %s", filename);
		sj_free(json);
		return NULL;
	}
	mesh = gf3d_mesh_load_obj(str);
	if (!mesh){
		slog("failed to parse obj data for model file %s", filename);
		sj_free(json);
		return NULL; // say why in gf3d_mesh_load_obj
	}
	str2 = sj_object_get_string(data,"texture");
	if(str2)
	{
		texture = gf3d_texture_load(str2);
		if(!texture) texture = model_manager.defultTexture;
	}
	else texture - model_manager.defaultTexture;
	model = model_new();
	if(!model)
	{
		slog("failed to get an empty space in memory for model %s",filename);
		return NULL;
	}
	model->mesh = mesh;
	model->texture = texture;
	gfc_line_cpy(model->filename,filename);
	return model;	




}
