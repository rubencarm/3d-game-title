#include "gf3d_pipeline.h"
#include "simple_logger.h"

#include "gf3d_mesh.h"
#include "gf3d_vgraphics.h"
#include "gf3d_obj_load.h"
#include "gf3d_buffers.h"
#include <vulkan/vulkan_core.h>

#define MESH_ATTRIBUTE_COUNT 3

typedef struct
{
	Uint32		meshCount;
	Mesh 		*meshList;
    VkDevice device;
    VkVertexInputAttributeDescription attributeDescriptions[MESH_ATTRIBUTE_COUNT];
    VkVertexInputBindingDescription bindingDescription;

}MeshManager;

static MeshManager mesh_manager = {0};
void gf3d_mesh_close(void);
ObjData *gf3d_obj_load_from_file(const char *filename) { return NULL; }

void gf3d_mesh_delete(Mesh *mesh) {}

void gf3d_mesh_primitive_delete(MeshPrimitive *prim) {}

void gf3d_mesh_manager_close(void) {
        if (mesh_manager.meshList)
        {
            free(mesh_manager.meshList);
        }
        memset(&mesh_manager, '0', sizeof(MeshManager));
}
void gf3d_mesh_init(Uint32 mesh_max) {


	if(mesh_manager.meshCount != 0)
	{
		slog("Cannot init mesh system, already initialized");
		return;
	}
	if(mesh_max == 0)
	{	
		slog("Cannot initialize mesh system for zero meshes");
		return;
	}

    mesh_manager.meshList = (Mesh *)gfc_allocate_array(sizeof(Mesh), mesh_max);
    if (!mesh_manager.meshList)
            return;
    mesh_manager.meshCount = mesh_max;
    mesh_manager.device = gf3d_vgraphics_get_default_logical_device();

	atexit(gf3d_mesh_close);
}

void *gf3d_mesh_get_by_filename(const char *filename)
{
	if(!filename) return NULL;
	int i;
	for(i = 0; i < mesh_manager.meshCount; i++){
		if(strlen(mesh_manager.meshList[i].filename) == 0) continue;
		if(gfc_strlcmp(mesh_manager.meshList[i].filename, filename) == 0)
		{
			return &mesh_manager.meshList[i];
		}
	}
	return NULL;
}

Mesh *gf3d_mesh_load_obj(const char *filename)
{
	Mesh *mesh;
	MeshPrimitive* prim;
	if(!filename) return NULL;

	mesh = gf3d_mesh_get_by_filename(filename);
	if(mesh)
	{
		mesh->_refCount++;
		return mesh;
	}
	mesh = gf3d_mesh_new();
	if(!mesh)
	{
		slog("failed to allocate new mesh");
		return NULL;
	}
	prim = gf3d_mesh_primitive_new();
	if(!prim){
		slog("failed to get a new primitive mesh for %s", filename);

	}
	prim->objData = gf3d_obj_load_from_file(filename);
	if(!prim->objData)
	{
		slog("failed to parse file %s for obj data", filename);
        gf3d_mesh_delete(mesh);
        gf3d_mesh_primitive_delete(prim);
		return NULL;
	}
	
}

int gf3d_mesh_primitive_buffer_create(MeshPrimitive *prim){

	void* data = NULL;
	Uint32 bufferSize = 0;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	if((!prim) || (!prim->objData)) return 0;

	// face buffers
	bufferSize = sizeof(Face) *prim->objData->face_count;

        gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           &stagingBuffer,
                           &stagingBufferMemory);
	vkMapMemory(mesh_manager.device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, prim->objData->outFace, (size_t) bufferSize);
	vkUnmapMemory(mesh_manager.device, stagingBufferMemory);

	gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prim->faceBuffer, &prim->faceBufferMemory);

	gf3d_buffer_copy(stagingBuffer, prim->faceBuffer, bufferSize);
	//vertex buffers
	bufferSize = sizeof(Face) * prim->objData->face_vert_count;

        gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           &stagingBuffer,
                           &stagingBufferMemory);
	vkMapMemory(mesh_manager.device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, prim->objData->faceVertices, (size_t) bufferSize);
	vkUnmapMemory(mesh_manager.device, stagingBufferMemory);

	gf3d_buffer_create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &prim->vertexBuffer, &prim->vertexBufferMemory);

	gf3d_buffer_copy(stagingBuffer, prim->vertexBuffer, bufferSize);

	vkDestroyBuffer(mesh_manager.device, stagingBuffer, NULL);
	vkFreeMemory(mesh_manager.device, stagingBufferMemory, NULL);

	return 1;
}
int gf3d_mesh_obj_buffer_create(Mesh *mesh)
{
	int i,c;
	int ret = 0;
	MeshPrimitive *prim;
	if(!mesh) return 0;
	prim = gf3d_mesh_primitive_new();
	if(!prim)
	{
		slog("failed to get new primitive for mesh");
		return 0;
	}
	if(gf3d_mesh_primitive_buffer_create(prim))
	{
		gfc_list_append(mesh->primitives,prim);
		return 1;
	}
	return 0;
}


void gf3d_mesh_close(void)
{
	int i;
	//go through list of meshes and free them all
	for(i = 0; i < mesh_manager.meshCount; i++){
		gf3d_mesh_free(&mesh_manager.meshList[i]);
	}
	free(mesh_manager.meshList);
	memset(&mesh_manager, 0, sizeof(MeshManager));
}

Mesh *gf3d_mesh_new(void){
	int i;

	for(i = 0; i < mesh_manager.meshCount; i++){
		if(mesh_manager.meshList[i]._refCount == 0)
		{
			mesh_manager.meshList[i].primitives = gfc_list_new();
			if(mesh_manager.meshList[i].primitives == NULL)
			{
				slog("cannot allocate more memory for a new mesh");
				return NULL;
			};
			mesh_manager.meshList[i]._refCount = 1;
			return &mesh_manager.meshList[i];
		}
	}
	return NULL;
}
void gf3d_mesh_primitive_free(MeshPrimitive* prim)
{
	if (prim->vertexBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(mesh_manager.device, prim->vertexBuffer, NULL);
	}
	if(prim->vertexBufferMemory != VK_NULL_HANDLE){
		vkFreeMemory(mesh_manager.device, prim->vertexBufferMemory, NULL);
	}
	if(prim->faceBuffer != VK_NULL_HANDLE){
		vkDestroyBuffer(mesh_manager.device, prim->faceBuffer, NULL);
	}
	if(prim->faceBufferMemory != VK_NULL_HANDLE){
		vkFreeMemory(mesh_manager.device, prim->faceBufferMemory, NULL);
	}
}


void gf3d_mesh_free(Mesh *mesh)
{
	int i,c;
	MeshPrimitive *prim;
	if(!mesh) return;
	c=gfc_list_count(mesh->primitives);
	for(i = 0; i < c; i++)
	{
		prim=gfc_list_nth(mesh->primitives,i);
		if(!prim) continue;
		gf3d_mesh_primitive_free(prim);
		
	}

}

/*eof@eol*/
/*

 * @brief initializes the mesh system / configures internal data about mesh based rendering
 * @param mesh_max the maximum allowed simultaneous meshes supported at once.  Must be > 0
 * @note keep in mind that many models will be comprised of multiple sub meshes.  So this number may need to be very large
void gf3d_mesh_init(Uint32 mesh_max);

 * @brief get a new empty model
 * @return NULL on error, or an empty model
Mesh *gf3d_mesh_new();

 * @brief load mesh data from an obj filename.
 * @note: currently only supporting obj files
 * @note this free's the intermediate data loaded from the obj file, no longer needed for most applications
 * @param filename the name of the file to load
 * @return NULL on error or Mesh data
Mesh *gf3d_mesh_load_obj(const char *filename);

 * @brief make an exact, but separate copy of the input mesh
 * @param in the mesh to duplicate
 * @return NULL on error, or a copy of in
Mesh *gf3d_mesh_copy(Mesh *in);

 * @brief move all of the vertices of the mesh by offset at the buffer level
 * @param in the mesh to move
 * @param offset how much to move it
 * @param rotation apply this rotation to the vertices and normals
void gf3d_mesh_move_vertices(Mesh *in, GFC_Vector3D offset,GFC_Vector3D rotation);

 * @brief allocate a zero initialized mesh primitive
 * @return NULL on error or the primitive
MeshPrimitive *gf3d_mesh_primitive_new();


 * @brief get the input attribute descriptions for mesh based rendering
 * @param count (optional, output) the number of attributes
 * @return a pointer to a vertex input attribute description array
VkVertexInputAttributeDescription * gf3d_mesh_get_attribute_descriptions(Uint32 *count);

 * @brief get the binding description for mesh based rendering
 * @return vertex input binding descriptions compatible with mesh data
VkVertexInputBindingDescription * gf3d_mesh_get_bind_description();

 * @brief free a mesh that has been loaded from memory
void gf3d_mesh_free(Mesh *mesh);

 * @brief needs to be called once at the beginning of each render frame
void gf3d_mesh_reset_pipes();

 * @brief called to submit all draw commands to the mesh pipelines
void gf3d_mesh_submit_pipe_commands();

 * @brief get the current command buffer for the mesh system
VkCommandBuffer gf3d_mesh_get_model_command_buffer();


 * @brief queue up a render for the current draw frame
 * @param mesh the mesh to render
 * @param pipe the pipeline to use
 * @param uboData the data to use to draw the mesh
 * @param texture texture data to use
void gf3d_mesh_queue_render(Mesh *mesh,Pipeline *pipe,void *uboData,Texture *texture);


 * @brief adds a mesh to the render pass rendered as an outline highlight
 * @note: must be called within the render pass
 * @param mesh the mesh to render
 * @param com the command pool to use to handle the request we are rendering with
void gf3d_mesh_render(Mesh *mesh,VkCommandBuffer commandBuffer, VkDescriptorSet * descriptorSet);

 * @brief render a mesh through a given pipeline
void gf3d_mesh_render_generic(Mesh *mesh,Pipeline *pipe,VkDescriptorSet * descriptorSet);

 * @brief create a mesh's internal buffers based on vertices
 * @param primitive the mesh primitive to populate
 * @note the primitive must have the objData set and it must have be organizes in buffer order
void gf3d_mesh_create_vertex_buffer_from_vertices(MeshPrimitive *primitive);

 * @brief get the pipeline that is used to render basic 3d meshes
 * @return NULL on error or the pipeline in question
Pipeline *gf3d_mesh_get_pipeline();

 * @brief given a model matrix and basic color, build the meshUBO needed to render a model
* @param modelMat the model Matriix
 * @param colorMod the color for the UBO
MeshUBO gf3d_mesh_get_ubo(
    GFC_Matrix4 modelMat,
    GFC_Color colorMod);
*/
