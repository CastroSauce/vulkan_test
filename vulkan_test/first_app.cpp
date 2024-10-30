#include "first_app.hpp"
#include "keyboard_movement_controller.hpp"
#include "lve_game_object.hpp"
#include "simple_render_system.hpp"
#include "lve_camera.hpp"
#include <stdexcept>
#include <glm.hpp>
#include <array>
#include <vector>
#include <chrono>
namespace lve {

    FirstApp::FirstApp() {
        loadGameObjects();
    }


    FirstApp::~FirstApp() {

    }

    void FirstApp::run() {
        SimpleRenderSystem simpleRenderSystem{ lveDevice, lveRenderer.getSwapChainRenderPass() };
        LveCamera camera{};
        camera.setViewDirection(glm::vec3(0.f), glm::vec3(.0f, .0f, 1.5f));

        auto viewerObject = LveGameObject::createObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();
            
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(lveWindow.getWindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = lveRenderer.getAspectRatio();
            //camera.setOrthographicProjection(-aspect,aspect,-1,1,-1,1);
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

            if (auto commandbuffer = lveRenderer.beginFrame()) {

                lveRenderer.beginSwapChainRenderPass(commandbuffer);


                simpleRenderSystem.renderGameObjects(commandbuffer, gameObjects,camera);
                lveRenderer.endSwapChainRenderPass(commandbuffer);
                lveRenderer.endFrame();


            }

        }

        vkDeviceWaitIdle(lveDevice.device());

    }

    // temporary helper function, creates a 1x1x1 cube centered at offset
    std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset) {
        LveModel::Builder modelBuilder{};
        modelBuilder.vertecies = {
            // left face (white)
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

            // right face (yellow)
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

            // top face (orange, remember y axis points down)
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

            // bottom face (red)
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

            // nose face (blue)
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

            // tail face (green)
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        };
        for (auto& v : modelBuilder.vertecies) {
            v.position += offset;
        }

        modelBuilder.indices = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                                12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

        return std::make_unique<LveModel>(device, modelBuilder);
    }


    void FirstApp::loadGameObjects() {

        //std::shared_ptr<LveModel> lveModel = createCubeModel(lveDevice, { .0f,.0f,.0f });
        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "C:/Users/Felix/source/repos/vulkan_test/vulkan_test/models/smooth_vase.obj");
        auto cube = LveGameObject::createObject();
        cube.model = lveModel;
        
        cube.transform.translation = { .0f,.0f, 1.5f};
        cube.transform.scale = { .5f,.5f,.5f };
        cube.transform.rotation.y = glm::radians(20.f);
        cube.transform.rotation.x = glm::radians(20.f);

        gameObjects.push_back(cube);
    }


}