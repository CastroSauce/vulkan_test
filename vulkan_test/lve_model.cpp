#include "lve_model.hpp"

//libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <iostream>

namespace lve {

	LveModel::LveModel(LveDevice& device, const LveModel::Builder& builder) : lveDevice{device} {
		createVertexBuffer(builder.vertecies);
		createIndexBuffer(builder.indices);
	}

	LveModel::~LveModel(){
		vkDestroyBuffer(lveDevice.device(), vertexBuffer, nullptr);
		vkFreeMemory(lveDevice.device(), vertexBufferMemory, nullptr);

		if (hasIndexBuffer) {
			vkDestroyBuffer(lveDevice.device(), indexBuffer, nullptr);
			vkFreeMemory(lveDevice.device(), indexBufferMemory, nullptr);
		}
	}


	void LveModel::createVertexBuffer(const std::vector<Vertex> &vertecies) {
		vertexCount = static_cast<uint32_t>(vertecies.size());
		VkDeviceSize bufferSize = sizeof(vertecies[0]) * vertexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		lveDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertecies.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(lveDevice.device(), stagingBufferMemory);


		lveDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer,
			vertexBufferMemory
		);

		lveDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(lveDevice.device(), stagingBufferMemory, nullptr);
	}
		
	
	void LveModel::createIndexBuffer(const std::vector<uint32_t>& indicies){
		indexCount = static_cast<uint32_t>(indicies.size());

		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) return;

		VkDeviceSize bufferSize = sizeof(indicies[0]) * indexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		lveDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indicies.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(lveDevice.device(), stagingBufferMemory);


		lveDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer,
			indexBufferMemory
		);

		lveDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(lveDevice.device(), stagingBufferMemory, nullptr);

	}

	std::unique_ptr<LveModel> LveModel::createModelFromFile(LveDevice& device, const std::string& filepath)
	{
		Builder builder{};
		builder.loadModel(filepath);

		std::cout << "Vertex count: " << builder.vertecies.size() << "\n";

		return std::make_unique<LveModel>(device, builder);
	}

	void LveModel::bind(VkCommandBuffer commandBuffer) {

		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32 );
		}


		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

	}
	void LveModel::draw(VkCommandBuffer commandBuffer) {

		if (hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		}

	}

	 std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions() {
		 std::vector<VkVertexInputBindingDescription> bindingDescription(1);
		 bindingDescription[0].binding = 0;
		 bindingDescription[0].stride = sizeof(Vertex);
		 bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		 return bindingDescription;
	}


	 std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getBindingAttributes() {
		 std::vector<VkVertexInputAttributeDescription> attributeDescription(2);

		 attributeDescription[0].binding = 0;
		 attributeDescription[0].location = 0;
		 attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		 attributeDescription[0].offset = offsetof(Vertex, position);		 
		 
		 attributeDescription[1].binding = 0;
		 attributeDescription[1].location = 1;
		 attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		 attributeDescription[1].offset = offsetof(Vertex, color);

		 return attributeDescription;

	 }

	 void LveModel::Builder::loadModel(const std::string &filepath) {
		 tinyobj::attrib_t attrib;
		 std::vector<tinyobj::shape_t>  shapes;
		 std::vector<tinyobj::material_t>  materials;
		 std::string warn, error;

		 if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, filepath.c_str())) {
			 throw std::runtime_error{ warn + error };
		 };

		 vertecies.clear();
		 indices.clear();

		 for (const auto& shape : shapes) {

			 for (const auto& index : shape.mesh.indices) {
				 Vertex vertex{};

				 if (index.vertex_index >= 0) {

					 vertex.position = {
						 attrib.vertices[3 * index.vertex_index + 0],
						 attrib.vertices[3 * index.vertex_index + 1],
						 attrib.vertices[3 * index.vertex_index + 2]
					 };

					 //auto colorIndex = 3 * index.vertex_index + 2;
					 vertex.color = { 1.f,1.f,1.f };

				/*	 if (colorIndex < attrib.colors.size()) {
						 vertex.color = {
							 attrib.colors[colorIndex + 0],
							 attrib.colors[colorIndex + 1],
							 attrib.colors[colorIndex + 2]
						 };
					 }
					 else {
					 }*/

				 }

				 if (index.normal_index >= 0) {

					 vertex.normal = {
						 attrib.normals[3 * index.normal_index + 0],
						 attrib.normals[3 * index.normal_index + 1],
						 attrib.normals[3 * index.normal_index + 2]
					 };
				 }

				 if (index.texcoord_index >= 0) {

					 vertex.uv = {
						 attrib.texcoords[2 * index.texcoord_index + 0],
						 attrib.texcoords[2 * index.texcoord_index + 1],
					 };
				 }

				 //vertecies.push_back(vertex);
			 }

		 }

	 }

}