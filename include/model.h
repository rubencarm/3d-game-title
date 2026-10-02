#ifndef __MODEL_H_
#define __MODEL_H_


#include "gf3d_mesh.h"

typedef struct
{
	GFC_Matrix4	model;
	GFC_Matrix4 	view;
	GFC_Matrix4	proj;
	GFC_Vector4D	color;
}ModelUBO;

typedef struct model_s {
	int			_refCount;
	Texture		*texture;
    Mesh *mesh;
    GFC_Line filename;
	VkDescriptorSet		*descriptorSet;

} Model;

void model_init_system(Uint32 modelCount);

#endif
