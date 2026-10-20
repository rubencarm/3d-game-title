#ifndef __MODEL_H_
#define __MODEL_H_

#include "gf3d_mesh.h"

typedef struct
{
	GFC_Matrix4	    model;
	GFC_Matrix4 	view;
	GFC_Matrix4	    proj;
	GFC_Vector4D	color;
}ModelUBO;

typedef struct {
	int			        _refCount;
	Texture		        *texture;
    Mesh                *mesh;
} Model;

void model_init_system(Uint32 modelCount);

Model* model_new(void);
Model* model_load_from_file(const char* filename);
Model* model_load(const char* filename);
void model_free(Model* model);
void model_close(void);
void model_queue_render(Model* model, GFC_Matrix4 modelMat, GFC_Color colorMod);

#endif
