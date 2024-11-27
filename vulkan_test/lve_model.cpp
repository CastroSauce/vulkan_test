#include "lve_model.hpp"
#include "lve_buffer.hpp"
//libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL


#include <cassert>
#include <string>
#include <unordered_map>
#include <iostream>

namespace lve {

	LveModel::LveModel(LveDevice& device, const LveModel::Builder& builder) : lveDevice{device} {
		createVertexBuffer(builder.vertecies);
		createIndexBuffer(builder.indices);
	}

	LveModel::~LveModel(){}


	void LveModel::createVertexBuffer(const std::vector<Vertex> &vertecies) {
		vertexCount = static_cast<uint32_t>(vertecies.size());
		VkDeviceSize bufferSize = sizeof(vertecies[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertecies[0]);

		LveBuffer stagingBuffer{
			lveDevice,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertecies.data());

		vertexBuffer = std::make_unique<LveBuffer>(
			lveDevice,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);


		lveDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);

	}
		
	
	void LveModel::createIndexBuffer(const std::vector<uint32_t>& indicies){
		indexCount = static_cast<uint32_t>(indicies.size());

		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) return;

		VkDeviceSize bufferSize = sizeof(indicies[0]) * indexCount;
		uint32_t indexSize = sizeof(indicies[0]);

		LveBuffer stagingBuffer{
			lveDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indicies.data());

		indexBuffer = std::make_unique<LveBuffer>(
			lveDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);


		lveDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
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
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}


		VkBuffer buffers[] = { vertexBuffer->getBuffer()};
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
		 std::vector<VkVertexInputAttributeDescription> attributeDescription{};

		 attributeDescription.push_back({ 0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, position) });
		 attributeDescription.push_back({ 1,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, color) });
		 attributeDescription.push_back({ 2,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, normal) });
		 attributeDescription.push_back({ 3,0,VK_FORMAT_R32G32_SFLOAT,offsetof(Vertex, uv) });

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

					 vertex.color = {
							 attrib.colors[3 * index.vertex_index + 0],
							 attrib.colors[3 * index.vertex_index + 1],
							 attrib.colors[3 * index.vertex_index + 2]
					 };

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

				 vertecies.push_back(vertex);
			 }

		 }

	 }

}