#ifndef __MODEL_H_
#define __MODEL_H_

typedef struct model_s {
	int			_refCount;
	Texture			*texture;
	Mesh			*mesh;
	VkDescriptorSet		*descriptorSet;

} Model;


#endif
