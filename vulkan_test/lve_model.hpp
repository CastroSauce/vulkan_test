#pragma once

#include "lve_device.hpp"
#include "glm.hpp"
#include "memory"
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE


namespace lve {
	class LveModel {

	public:
		struct Vertex {
			glm::vec3 position;
			glm::vec3 color;
			glm::vec3 normal;
			glm::vec2 uv;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getBindingAttributes();
		};


		struct Builder {
			std::vector<Vertex> vertecies{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string& filepath);
		};
		
	LveModel(LveDevice &device, const LveModel::Builder& builder);
	~LveModel();

	LveModel(const LveModel &) = delete;
	LveModel& operator=(const LveModel&) = delete;

	static std::unique_ptr<LveModel> createModelFromFile(LveDevice& device, const std::string& filepath);

	void bind(VkCommandBuffer commandBuffer);
	void draw(VkCommandBuffer commandBuffer);


	private:
		void createVertexBuffer(const std::vector<Vertex>& vertecies);
		void createIndexBuffer(const std::vector<uint32_t>& indicies);

		LveDevice& lveDevice;

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;		
		
		bool hasIndexBuffer = false;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
		uint32_t indexCount;
	};
}